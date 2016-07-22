#include "remotecontrol.h"

#include <model/sqldatabase.h>
#include <cover.h>
#include <filehelper.h>
#include <mediaplayer.h>

#include <QDataStream>
#include <QTcpSocket>

#include <QtDebug>

RemoteControl::RemoteControl(MediaPlayer *mediaPlayer, int port, QObject *parent)
	: QObject(parent)
	, _mediaPlayer(mediaPlayer)
	, _port(port)
	, _tcpServer(new QTcpServer(this))
{
	connect(_tcpServer, &QTcpServer::newConnection, this, &RemoteControl::initializeConnection);
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
}

void RemoteControl::decodeResponseFromClient()
{
	QDataStream in;
	in.setDevice(_tcpSocket);
	in.setVersion(QDataStream::Qt_5_7);
	in.startTransaction();

	int command;
	QByteArray value;
	in >> command;
	in >> value;

	if (!in.commitTransaction()) {
		qDebug() << Q_FUNC_INFO << "commitTransaction failed";
		return;
	}

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
	case CMD_Volume: {
		qreal v = *reinterpret_cast<const qreal*>(value.data());
		qDebug() << Q_FUNC_INFO << "CMD_Volume" << v;
		_mediaPlayer->setVolume(v);
		break;
	}
	case CMD_Playlists:
		this->sendPlaylists();
		break;
	}
}

void RemoteControl::initializeConnection()
{
	qDebug() << Q_FUNC_INFO;
	QByteArray block;
	QDataStream out(&block, QIODevice::ReadWrite);
	out.setVersion(QDataStream::Qt_5_7);
	out << CMD_Connection;
	out << QByteArray("Hello from Miam-Player!");

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

	this->sendVolume(_mediaPlayer->volume());
}

void RemoteControl::mediaPlayerStatedChanged(QMediaPlayer::State state)
{
	if (!_tcpSocket) {
		return;
	}
	QByteArray block;
	QDataStream out(&block, QIODevice::ReadWrite);
	out.setVersion(QDataStream::Qt_5_7);
	out << CMD_State;
	if (state == QMediaPlayer::PlayingState) {
		qDebug() << "cmd:state:playing";
		out << QByteArray("playing");
	} else if (state == QMediaPlayer::PausedState) {
		qDebug() << "cmd:state:paused";
		out << QByteArray("paused");
	}
	_tcpSocket->write(block);
}

void RemoteControl::sendPlaylists() const
{
	if (!_tcpSocket) {
		return;
	}
	QByteArray block;
	QDataStream out(&block, QIODevice::ReadWrite);
	out.setVersion(QDataStream::Qt_5_7);
	out << CMD_Playlists;

	SqlDatabase db;
	auto playlists = db.selectPlaylists();
	out << playlists.size();
	for (PlaylistDAO p : playlists) {
		out << p.title();
	}

	_tcpSocket->write(block);
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
	out.setVersion(QDataStream::Qt_5_7);
	out << CMD_Track;
	TrackDAO dao = db.selectTrackByURI(track);
	//int daoSize = dao.uri().size() + dao.artistAlbum().size() + dao.album().size() + dao.title().size() + dao.trackNumber().size();
	//out << daoSize;
	//out << dao;
	out << dao.uri();
	out << dao.artistAlbum();
	out << dao.album();
	out << dao.title();
	out << dao.trackNumber();
	_tcpSocket->write(block);
	qDebug() << Q_FUNC_INFO << "cmd:track, " << dao.uri() << dao.artistAlbum() << dao.album() << dao.title();

	// Send cover if any
	/*Cover *cover = db.selectCoverFromURI(track);
	if (cover) {
		QByteArray block;
		QDataStream out(&block, QIODevice::ReadWrite);
		out.setVersion(QDataStream::Qt_5_7);
		out << CMD_Cover;
		QByteArray c;
		c.append(cover->byteArray());
		out << c.size();
		out << c;
		auto r = _tcpSocket->write(block);
		qDebug() << Q_FUNC_INFO << "cmd:cover, cover size:" << c.size() << ", bytes written" << r;
	}*/
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
	//QByteArray ba;
	//ba.append(reinterpret_cast<const char*>(&volume), sizeof(volume));
	out << ba;
	auto r = _tcpSocket->write(data);
	qDebug() << Q_FUNC_INFO << "sizeof(CMD):" << sizeof(Command) << "cmd:volume, bytes written" << r << ", volume:" << volume << ba.toStdString().data();
}
