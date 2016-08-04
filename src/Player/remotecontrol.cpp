#include "remotecontrol.h"

#include <model/sqldatabase.h>
#include <cover.h>
#include <filehelper.h>
#include <mediaplayer.h>

#include <QDataStream>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QTimer>
#include <QTcpSocket>

#include <QtDebug>

RemoteControl::RemoteControl(MediaPlayer *mediaPlayer, int port, QObject *parent)
	: QObject(parent)
	, _mediaPlayer(mediaPlayer)
	, _port(port)
	, _tcpServer(new QTcpServer(this))
	, _udpSocket(new QUdpSocket(this))
	, _timer(new QTimer(this))
{
	connect(_tcpServer, &QTcpServer::newConnection, this, &RemoteControl::initializeConnection);
	_timer->setSingleShot(true);
	_timer->setInterval(1000);
}

RemoteControl::~RemoteControl()
{
	_tcpServer->close();
}

void RemoteControl::changeServerPort(int port)
{
	_tcpServer->close();
	_port = port;
	_tcpServer->listen(QHostAddress::Any, _port);
}

void RemoteControl::startServer()
{
	_tcpServer->listen(QHostAddress::Any, _port);
	_udpSocket->bind(_port, QUdpSocket::ShareAddress);

	connect(_udpSocket, &QUdpSocket::readyRead, this, [=]() {
		qDebug() << Q_FUNC_INFO;
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
			qDebug() << Q_FUNC_INFO << "sending server infos:" << host << "to client:" << client;
			_udpSocket->writeDatagram(hostByteArray, QHostAddress(client), _port);
		}
	});
}

void RemoteControl::decodeResponseFromClient()
{
	QDataStream in;
	in.setDevice(_tcpSocket);
	in.setVersion(QDataStream::Qt_5_5);

	int command;
	QByteArray value;
	in >> command;
	in >> value;

	switch (command) {
	case CMD_Playback: {
		QString string = QString::fromStdString(value.toStdString());
		if (string == "skip-backward") {
			_mediaPlayer->skipBackward();
		} else if (string == "play-pause") {
			_mediaPlayer->togglePlayback();
		} else if (string == "skip-forward") {
			_mediaPlayer->skipForward();
		}
		break;
	}
	case CMD_Position: {
		qreal p = QString::fromStdString(value.toStdString()).toFloat();
		qDebug() << Q_FUNC_INFO << "CMD_Position" << p;
		_mediaPlayer->seek(p);
		break;
	}
	case CMD_Volume: {
		qreal v = QString::fromStdString(value.toStdString()).toFloat();
		qDebug() << Q_FUNC_INFO << "CMD_Volume" << v;
		_mediaPlayer->setVolume(v);
		break;
	}
	case CMD_ActivePlaylists:
		this->sendActivePlaylists();
		break;
	case CMD_AllPlaylists:
		this->sendAllPlaylists();
		break;
	}
}

void RemoteControl::initializeConnection()
{
	qDebug() << Q_FUNC_INFO;
	QByteArray block;
	QDataStream out(&block, QIODevice::ReadWrite);
	out.setVersion(QDataStream::Qt_5_5);
	out << CMD_Connection;
	out << QHostInfo::localHostName();

	_tcpSocket = _tcpServer->nextPendingConnection();
	connect(_tcpSocket, &QAbstractSocket::disconnected, this, [=]() {
		this->disconnect(_tcpSocket);
		_mediaPlayer->disconnect(this);
		_tcpSocket->deleteLater();
	});
	auto r = _tcpSocket->write(block);
	qDebug() << Q_FUNC_INFO << "sizeof(CMD):" << sizeof(Command) << "cmd:connect, bytes written" << r;

	connect(_tcpSocket, &QAbstractSocket::readyRead, this, &RemoteControl::decodeResponseFromClient);
	connect(_tcpSocket, &QTcpSocket::bytesWritten, this, [=](qint64 bytes) {
		qDebug() << "QTcpSocket::bytesWritten" << bytes;
	});
	connect(_mediaPlayer, &MediaPlayer::volumeChanged, this, &RemoteControl::sendVolume);
	connect(_mediaPlayer, &MediaPlayer::stateChanged, this, &RemoteControl::mediaPlayerStatedChanged);
	connect(_mediaPlayer, &MediaPlayer::currentMediaChanged, this, &RemoteControl::sendTrackInfos);
	connect(_mediaPlayer, &MediaPlayer::positionChanged, this, &RemoteControl::sendPosition);

	this->sendVolume(_mediaPlayer->volume());
	if (_mediaPlayer->state() == QMediaPlayer::PlayingState) {
		this->sendTrackInfos(_mediaPlayer->currentTrack());
	}
}

void RemoteControl::mediaPlayerStatedChanged(QMediaPlayer::State state)
{
	if (!_tcpSocket) {
		return;
	}
	QByteArray block;
	QDataStream out(&block, QIODevice::ReadWrite);
	out.setVersion(QDataStream::Qt_5_5);
	out << CMD_State;
	switch (state) {
	case QMediaPlayer::PlayingState:
		out << QString("playing");
		break;
	case QMediaPlayer::PausedState:
		out << QString("paused");
		break;
	case QMediaPlayer::StoppedState:
		out << QString("stopped");
		break;
	}
	_tcpSocket->write(block);
}

void RemoteControl::sendActivePlaylists() const
{
	if (!_tcpSocket) {
		return;
	}
	QByteArray block;
	QDataStream out(&block, QIODevice::ReadWrite);
	out.setVersion(QDataStream::Qt_5_5);
	out << CMD_ActivePlaylists;

	/*SqlDatabase db;
	auto playlists = db.selectPlaylists();
	out << playlists.size();
	for (PlaylistDAO p : playlists) {
		out << p.title();
	}*/

	_tcpSocket->write(block);
}

void RemoteControl::sendAllPlaylists() const
{
	if (!_tcpSocket) {
		return;
	}
	QByteArray block;
	QDataStream out(&block, QIODevice::ReadWrite);
	out.setVersion(QDataStream::Qt_5_5);
	out << CMD_AllPlaylists;

	SqlDatabase db;
	auto playlists = db.selectPlaylists();
	out << playlists.size();
	for (PlaylistDAO p : playlists) {
		out << p.title();
	}

	_tcpSocket->write(block);
}

void RemoteControl::sendPosition(qint64 pos, qint64 duration)
{
	if (!_tcpSocket) {
		return;
	}
	if (_timer->isActive()) {
		return;
	}
	_timer->start();

	QByteArray block;
	QDataStream out(&block, QIODevice::ReadWrite);
	out.setVersion(QDataStream::Qt_5_5);
	out << CMD_Position;
	out << pos;
	out << duration;

	_tcpSocket->write(block);
	qDebug() << Q_FUNC_INFO << "cmd:position, " << pos << duration;
}

void RemoteControl::sendTrackInfos(const QString &track)
{
	if (!_tcpSocket) {
		return;
	}

	SqlDatabase db;

	// Send track info
	QByteArray block;
	QDataStream out(&block, QIODevice::ReadWrite);
	out.setVersion(QDataStream::Qt_5_5);
	out << CMD_Track;
	TrackDAO dao = db.selectTrackByURI(track);
	out << dao.uri();
	out << dao.artistAlbum();
	out << dao.album();
	out << dao.title();
	out << dao.trackNumber();
	out << dao.rating();
	_tcpSocket->write(block);

	// Send cover if any
	Cover *cover = db.selectCoverFromURI(track);
	if (cover) {
		QByteArray block;
		QDataStream out(&block, QIODevice::ReadWrite);
		out.setVersion(QDataStream::Qt_5_5);
		out << CMD_Cover;
		QByteArray c;
		c.append(cover->byteArray());
		// First, send cover size, then the byteArray (which also contains the size)
		out << c.size();
		out << c;
		auto r = _tcpSocket->write(block);
		qDebug() << Q_FUNC_INFO << "cmd:cover, cover size:" << c.size() << ", bytes written" << r;
	}
}

void RemoteControl::sendVolume(qreal volume)
{
	qDebug() << Q_FUNC_INFO << volume;
	if (!_tcpSocket) {
		qDebug() << Q_FUNC_INFO << "Cannot send volume !";
		return;
	}
	QByteArray data;
	QDataStream out(&data, QIODevice::ReadWrite);
	out << CMD_Volume;
	QByteArray ba;
	ba.append(QString::number(volume));
	out << ba;
	_tcpSocket->write(data);
	qDebug() << Q_FUNC_INFO << "cmd:volume" << volume;
}
