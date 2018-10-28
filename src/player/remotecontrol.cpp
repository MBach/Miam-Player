#include "remotecontrol.h"

#include <model/sqldatabase.h>
#include <cover.h>
#include <filehelper.h>
#include <mediaplayer.h>
#include <abstractviewplaylists.h>

#include <QDataStream>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QTimer>
#include <QWebSocket>

#include <QtDebug>

RemoteControl::RemoteControl(AbstractView *currentView, int port, QObject *parent)
	: QObject(parent)
	, _currentView(currentView)
	, _port(port)
	, _webSocketServer(new QWebSocketServer("Miam-Player WebSocket Server", QWebSocketServer::NonSecureMode, this))
	, _udpSocket(new QUdpSocket(this))
	, _timer(new QTimer(this))
{
	connect(_webSocketServer, &QWebSocketServer::newConnection, this, &RemoteControl::initializeConnection);
	_timer->setSingleShot(true);
	_timer->setInterval(1000);
}

RemoteControl::~RemoteControl()
{
	_webSocketServer->close();
}

void RemoteControl::changeServerPort(int port)
{
	_webSocketServer->close();
	_port = port;
	_webSocketServer->listen();
}

void RemoteControl::startServer()
{
	auto b = _webSocketServer->listen(QHostAddress::Any, _port);
	_udpSocket->bind(_port, QUdpSocket::ShareAddress);

	connect(_udpSocket, &QUdpSocket::readyRead, this, [=]() {
		if (_udpSocket->hasPendingDatagrams()) {

			QByteArray clientHost;
			clientHost.resize(_udpSocket->pendingDatagramSize());
			_udpSocket->readDatagram(clientHost.data(), clientHost.size());
			QString client;
			client = QString::fromUtf8(clientHost);
			if (client.isEmpty()) {
				return;
			}
			//_udpSocket->abort();

			QString host;
			for (const QHostAddress &address : QNetworkInterface::allAddresses()) {
				if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost)) {
					host = address.toString();
					break;
				}
			}

			QByteArray hostByteArray;
			hostByteArray.append(host);
			_udpSocket->writeDatagram(hostByteArray, QHostAddress(client), _port);
		}
	});
}

void RemoteControl::decodeResponseFromClient(const QString &message)
{
	if (message.isEmpty()) {
		return;
	}

	QStringList args = message.split(QChar::Null);
	if (args.count() < 2) {
		return;
	}

	int command = args.first().toInt();

	switch (command) {
	case CMD_Playback: {

		QString actionControl = args.at(1);
		if (actionControl == "skip-backward") {
			_currentView->mediaPlayerControl()->skipBackward();
		} else if (actionControl == "play-pause") {
			_currentView->mediaPlayerControl()->togglePlayback();
		} else if (actionControl == "skip-forward") {
			_currentView->mediaPlayerControl()->skipForward();
		}
		break;
	}
	case CMD_Position: {
		qreal position = args.at(1).toFloat();
		_currentView->mediaPlayerControl()->mediaPlayer()->seek(position);
		break;
	}
	case CMD_Volume: {
		qreal volume = args.at(1).toFloat();
		_currentView->mediaPlayerControl()->mediaPlayer()->setVolume(volume);
		break;
	}
	case CMD_ActivePlaylists:
		this->sendActivePlaylists();
		break;
	case CMD_AllPlaylists:
		this->sendAllPlaylists();
		break;
	case CMD_LoadActivePlaylist: {
		int index = args.at(1).toInt();
		this->sendActivePlaylist(index);
		break;
	}
	}
}

void RemoteControl::initializeConnection()
{
	_webSocket = _webSocketServer->nextPendingConnection();
	connect(_webSocket, &QWebSocket::disconnected, this, [=]() {
		this->disconnect(_webSocket);
		_currentView->mediaPlayerControl()->mediaPlayer()->disconnect(this);
		_webSocket->deleteLater();
	});

	QStringList args;
	args << QString::number(CMD_Connection);
	args << QHostInfo::localHostName();
	_webSocket->sendTextMessage(args.join(QChar::Null));

	connect(_webSocket, &QWebSocket::textMessageReceived, this, &RemoteControl::decodeResponseFromClient);
	connect(_currentView->mediaPlayerControl()->mediaPlayer(), &MediaPlayer::volumeChanged, this, &RemoteControl::sendVolume);
	connect(_currentView->mediaPlayerControl()->mediaPlayer(), &MediaPlayer::stateChanged, this, &RemoteControl::mediaPlayerStatedChanged);
	connect(_currentView->mediaPlayerControl()->mediaPlayer(), &MediaPlayer::currentMediaChanged, this, &RemoteControl::sendTrackInfos);
	connect(_currentView->mediaPlayerControl()->mediaPlayer(), &MediaPlayer::positionChanged, this, &RemoteControl::sendPosition);

	this->sendVolume(_currentView->mediaPlayerControl()->mediaPlayer()->volume());
	if (_currentView->mediaPlayerControl()->mediaPlayer()->state() == QMediaPlayer::PlayingState) {
		this->sendTrackInfos(_currentView->mediaPlayerControl()->mediaPlayer()->currentTrack());
	}
}

void RemoteControl::mediaPlayerStatedChanged(QMediaPlayer::State state)
{
	if (!_webSocket) {
		return;
	}
	QStringList args;
	args << QString::number(CMD_State);
	args << QString::number(state);

	_webSocket->sendTextMessage(args.join(QChar::Null));
}

void RemoteControl::sendActivePlaylist(int index)
{
	if (!_webSocket) {
		return;
	}

	QStringList args;
	args << QString::number(CMD_LoadActivePlaylist);

	if (_currentView && _currentView->viewProperty(Settings::VP_PlaylistFeature)) {
		AbstractViewPlaylists *playlistsView = qobject_cast<AbstractViewPlaylists*>(_currentView);
		MediaPlaylist *mp = playlistsView->playlists().at(index);

		// Send track count first, then all tracks in a big "blob"
		for (int i = 0; i < mp->mediaCount(); i++) {
			FileHelper fh(mp->media(i));
			//out << fh.title() << fh.trackNumber() << fh.album() << fh.artistAlbum() << fh.year() << QString::number(fh.rating());
			/*tracks.append(fh.title());
			tracks.append(fh.trackNumber());
			tracks.append(fh.album());
			tracks.append(fh.artistAlbum());
			tracks.append(fh.year());
			tracks.append(QString::number(fh.rating()));*/
		}
		//out << tracks.size();
		//out << tracks;
		_webSocket->sendTextMessage(args.join(QChar::Null));
	}
}

void RemoteControl::sendActivePlaylists() const
{
	if (!_webSocket) {
		return;
	}

	QStringList args;
	args << QString::number(CMD_ActivePlaylists);

	if (_currentView && _currentView->viewProperty(Settings::VP_PlaylistFeature)) {
		AbstractViewPlaylists *playlistsView = qobject_cast<AbstractViewPlaylists*>(_currentView);
		for (MediaPlaylist *mp : playlistsView->playlists()) {
			args << mp->title();
		}
	}
	_webSocket->sendTextMessage(args.join(QChar::Null));
}

void RemoteControl::sendAllPlaylists() const
{
	if (!_webSocket) {
		return;
	}

	QStringList args;
	args << QString::number(CMD_AllPlaylists);

	for (PlaylistDAO p : SqlDatabase().selectPlaylists()) {
		args << p.title();
	}
	_webSocket->sendTextMessage(args.join(QChar::Null));
}

void RemoteControl::sendPosition(qint64 pos, qint64 duration)
{
	if (!_webSocket) {
		return;
	}
	if (_timer->isActive()) {
		return;
	}
	_timer->start();

	QStringList args = { QString::number(CMD_Position), QString::number(pos), QString::number(duration) };
	_webSocket->sendTextMessage(args.join(QChar::Null));
}

void RemoteControl::sendTrackInfos(const QString &track)
{
	if (!_webSocket) {
		return;
	}

	QStringList args;
	args << QString::number(CMD_Track);

	SqlDatabase db;
	TrackDAO dao = db.selectTrackByURI(track);
	args << dao.uri();
	args << dao.artistAlbum();
	args << dao.album();
	args << dao.title();
	args << dao.trackNumber();
	args << QString::number(dao.rating());
	_webSocket->sendTextMessage(args.join(QChar::Null));

	// Send cover if any
	if (Cover *cover = db.selectCoverFromURI(track)) {
		_webSocket->sendBinaryMessage(cover->byteArray());
	}
}

void RemoteControl::sendVolume(qreal volume)
{
	if (!_webSocket) {
		return;
	}

	QStringList args = { QString::number(CMD_Volume), QString::number(volume) };
	_webSocket->sendTextMessage(args.join(QChar::Null));
}
