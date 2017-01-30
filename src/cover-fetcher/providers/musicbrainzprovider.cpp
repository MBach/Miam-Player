#include "musicbrainzprovider.h"

#include <QDir>
#include <QGroupBox>
#include <QStandardPaths>
#include <QXmlStreamReader>

#include <QtDebug>

MusicBrainzProvider::MusicBrainzProvider(QNetworkAccessManager *parent)
	: CoverArtProvider(parent)
{}

QUrl MusicBrainzProvider::query(const QString &artist, const QString &)
{
	return QUrl("http://musicbrainz.org/ws/2/release-group/?query=artist:%22" + artist + "%22;limit=100");
}

QUrl MusicBrainzProvider::album(const QString &albumId)
{
	return QUrl("http://coverartarchive.org/release-group/" + albumId + "/front");
}

void MusicBrainzProvider::dispatchReply(QNetworkReply *reply)
{
	// Dispatch request
	Fetch_Operations fo = (Fetch_Operations)reply->property("requestType").toInt();
	switch (fo) {
	//case FO_GetReleases:
	//	qDebug() << Q_FUNC_INFO << "Current fetch operation GetReleases" << reply->url();
	//	this->fetchReleases(reply->readAll());
	//	break;
	case FO_DownloadCover:
		qDebug() << Q_FUNC_INFO << "Current fetch operation DownloadCover" << reply->url();
		this->downloadCover(reply->readAll(), reply);
		break;
	case FO_Search:
		qDebug() << Q_FUNC_INFO << "Current fetch operation Search" << reply->url();
		this->fetchReleases(reply->property("album").toString(), reply->readAll());
		break;
	default:
		break;
	}
}

void MusicBrainzProvider::fetchReleases(const QString &album, const QByteArray &ba)
{
	QXmlStreamReader xml(ba);

	// Album Text -> Album ID
	QMap<QString, QString> map;
	while(!xml.atEnd() && !xml.hasError()) {

		QXmlStreamReader::TokenType token = xml.readNext();

		// Parse start elements
		if (token == QXmlStreamReader::StartElement) {
			if (xml.name() == "release-group") {
				if (xml.attributes().hasAttribute("id")) {
					QStringRef sr = xml.attributes().value("id");
					if (xml.readNextStartElement() && xml.name() == "title") {
						map.insert(xml.readElementText().toLower(), sr.toString());
					}
				}
			}
		}
	}

	QMapIterator<QString, QString> it(map);
	/// Complexity: should be improved!
	while (it.hasNext()) {
		it.next();
		if (uiLevenshteinDistance(album.toLower().toStdString(), it.key().toStdString()) < 4 ||
				it.key().contains(album.toLower()) || album.toLower().contains(it.key())) {
			qDebug() << "uiLevenshteinDistance: get the covert art for" << album << it.key() << it.value();
			/// FIXME: find a way to get the 500px thumbnail and to automatically download the large one after
			QUrl url = this->album(it.value());
			QNetworkRequest request(url);
			request.setHeader(QNetworkRequest::UserAgentHeader, "MiamPlayer/0.8.1 ( http://www.miam-player.org/ )" );
			QNetworkReply *reply = _manager->get(request);
			reply->setProperty("type", this->type());
			reply->setProperty("requestType", FO_DownloadCover);
			reply->setProperty("album", album);
			break;
		}
	}
}

// Compute Levenshtein Distance
// Martin Ettl, 2012-10-05
/** Levenshtein distance is a string metric for measuring the difference between two sequences. */
size_t MusicBrainzProvider::uiLevenshteinDistance(const std::string &s1, const std::string &s2)
{
	const size_t m(s1.size());
	const size_t n(s2.size());

	if (m == 0) return n;
	if (n == 0) return m;

	size_t *costs = new size_t[n + 1];

	for (size_t k = 0; k <= n; k++) costs[k] = k;

	size_t i = 0;
	for (std::string::const_iterator it1 = s1.begin(); it1 != s1.end(); ++it1, ++i) {
		costs[0] = i+1;
		size_t corner = i;

		size_t j = 0;
		for (std::string::const_iterator it2 = s2.begin(); it2 != s2.end(); ++it2, ++j) {
			size_t upper = costs[j+1];
			if (*it1 == *it2) {
				costs[j+1] = corner;
			} else {
				size_t t(upper < corner ? upper : corner);
				costs[j+1] = (costs[j] < t ? costs[j] : t) + 1;
			}
			corner = upper;
		}
	}

	size_t result = costs[n];
	delete [] costs;
	return result;
}

void MusicBrainzProvider::downloadCover(QByteArray ba, QNetworkReply *reply)
{
	QString album = reply->property("album").toString();

	// In case we don't get the picture at the first attempt, try again
	QVariant redirectionTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
	if (redirectionTarget.isNull()) {

		// The current cover has been downloaded to a temporary location, the lists can be populated
		//QPixmap pixmap;

		// It's possible to have a valid release but without cover yet :(
		//if (pixmap.loadFromData(ba)) {
		//	emit aboutToCreateCover(album, pixmap);
		//}
		emit aboutToCreateCover(album, ba);
	} else {
		QUrl newUrl = reply->url().resolved(redirectionTarget.toUrl());
		QNetworkReply *reply = _manager->get(QNetworkRequest(newUrl));
		reply->setProperty("type", type());
		reply->setProperty("requestType", FO_DownloadCover);
		reply->setProperty("album", album);
	}
}
