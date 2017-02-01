#include "requestpool.h"

#include "mbrelease.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QtDebug>

int RequestPool::_maxRequestPerSecond = 3;
int RequestPool::_nbRequestsToAcoustIdServer = 0;

RequestPool::RequestPool(QObject *parent)
	: QObject(parent), _timer(new QTimer(this))
{
	_timer->setSingleShot(true);
	connect(&_nam, &QNetworkAccessManager::finished, this, &RequestPool::dispatchReply);
	connect(_timer, &QTimer::timeout, this, [=]() {

		int n = qMin(_pool.count(), _maxRequestPerSecond);
		for (int i = 0; i < n; i++) {
			Quadruplet q = _pool.takeFirst();
			this->add(q.track, q.request, q.urlQuery, q.trackDuration);
		}
		if (!_pool.isEmpty()) {
			qDebug() << "there are some requests remaining, restarting timer";
			_timer->start(1000);
		}
	});
}

void RequestPool::add(const QString &track, const QNetworkRequest &request, const QUrlQuery &urlQuery, int trackDuration)
{
	if (_nbRequestsToAcoustIdServer > _maxRequestPerSecond) {
		_pool << Quadruplet(track, request, urlQuery, trackDuration);
		if (!_timer->isActive()) {
			qDebug() << Q_FUNC_INFO << "too much requests, waiting for 1s";
			_timer->start(1000);
		}
	} else {
		QNetworkReply* reply = _nam.post(request, urlQuery.query(QUrl::FullyEncoded).toUtf8());
		reply->setProperty("duration", trackDuration);
		reply->setProperty("track", track);
		_nbRequestsToAcoustIdServer++;
		_map.insert(track, reply);
	}
}

void RequestPool::dispatchReply(QNetworkReply *reply)
{
	_nbRequestsToAcoustIdServer--;
	QByteArray ba(reply->readAll());
	qDebug() << Q_FUNC_INFO << ba;
	int duration = reply->property("duration").toInt();
	QString absFilePath = reply->property("track").toString();
	_map.remove(absFilePath);

	QJsonDocument doc = QJsonDocument::fromJson(ba);
	if (!doc.isObject()) {
		qDebug() << Q_FUNC_INFO << "not an object?" << doc.isArray();
	}
	QJsonObject o = doc.object();

	if (o.value("status").toString() != "ok") {
		qDebug() << Q_FUNC_INFO << "status is not ok :(";
		return;
	}
	// Results are sorted by score
	QJsonArray results = o.value("results").toArray();
	if (results.isEmpty()) {
		qDebug() << Q_FUNC_INFO << "result list is empty :(";
		return;
	}

	// Even for the highest score, we need to extract the right MBID for the track
	// By adding more information in input request, like "releasegroups", the returned json will tell us if the track has been included
	// in compilation, singles, etc
	QJsonObject v = results.at(0).toObject();
	QString acoustID = v.value("id").toString();
	double score = v.value("score").toDouble();
	QJsonArray recordings = v.value("recordings").toArray();

	//qDebug() << Q_FUNC_INFO << "score:" << score << "acoustID:" << acoustID;
	QList<QJsonObject> potentialReleaseGroupList;
	for (int i = 0; i < recordings.count(); i++) {
		QJsonObject recording = recordings.at(i).toObject();
		if (recording.contains("duration")) {
			int d = recording.value("duration").toInt();
			if (d == duration) {
				potentialReleaseGroupList << recording;
			}
		}
	}

	// Iterate only over a subset of JSON
	// At this point, the potential list of values contains a list of releasegroups that we need to filter (exclude single, compilation)
	// Except if we have only one item!

	QString mbReleaseGroupId;
	//bool singleRelease = (potentialReleaseGroupList.count() == 1);
	for (int i = 0; i < potentialReleaseGroupList.count(); i++) {
		QJsonArray releasegroups = potentialReleaseGroupList.at(i).value("releasegroups").toArray();
		for (int j = 0; j < releasegroups.count(); j++) {
			QJsonObject releaseGroup = releasegroups.at(j).toObject();
			//qDebug() << Q_FUNC_INFO << "releaseGroup" << releaseGroup;
			if (releaseGroup.value("type") == "Album" && !releaseGroup.contains("secondarytypes")) {
				QJsonObject jArtist = releaseGroup.value("artists").toArray().first().toObject();
				MusicBrainz::Artist artist(this);
				artist.name = jArtist.value("name").toString();
				artist.id = jArtist.value("id").toString();

				QJsonArray releases = releaseGroup.value("releases").toArray();
				for (int k = 0; k < releases.count(); k++) {
					QJsonObject release = releases.at(k).toObject();

					MusicBrainz::Release mbRelease(this);
					mbRelease.releaseGroupId = mbReleaseGroupId;
					mbRelease.id = release.value("id").toString();
					mbRelease.artist = artist;
					mbRelease.trackCount = release.value("track_count").toInt();
					mbRelease.title = release.value("title").toString();
					mbRelease.country = release.value("country").toString();
					mbRelease.year = release.value("date").toObject().value("year").toInt();

					QJsonArray mediums = release.value("mediums").toArray();
					if (!mediums.isEmpty()) {
						QJsonObject medium = mediums.at(0).toObject();
						mbRelease.format = medium.value("format").toString();
						mbRelease.disc = medium.value("position").toInt();

						MusicBrainz::Track mbTrack(this);
						QJsonObject jTrack = medium.value("tracks").toArray().at(0).toObject();
						mbTrack.id = jTrack.value("id").toString();
						mbTrack.position = jTrack.value("position").toInt();
						mbTrack.title = jTrack.value("title").toString();
						QJsonObject jTrackArtist = jTrack.value("artists").toArray().at(0).toObject();
						QString trackArtist = jTrackArtist.value("artist").toString();
						if (mbRelease.artist.name != trackArtist) {
							mbTrack.artist->name = trackArtist;
							mbTrack.artist->id = jTrackArtist.value("id").toString();
						}
						mbRelease.tracks.insert(absFilePath, mbTrack);
					}
					emit releaseFound(mbRelease);
				}
				break;
			}
		}
	}
	qDebug() << Q_FUNC_INFO << acoustID << score << mbReleaseGroupId;

	// Call WebService?
	//http://musicbrainz.org/ws/2/recording/{mbReleaseId}?inc=artist-credits%2Breleases%2Bmedia&fmt=json
	if (_map.isEmpty()) {
		emit tracksAnalyzed();
	}
}
