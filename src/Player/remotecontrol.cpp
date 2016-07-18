#include "remotecontrol.h"

#include <model/sqldatabase.h>
#include <cover.h>
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
	case CMD_Volume:
		qreal v = *reinterpret_cast<const qreal*>(value.data());
		qDebug() << Q_FUNC_INFO << "CMD_Volume" << v;
		_mediaPlayer->setVolume(v);
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
	_tcpSocket->flush();
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
	_tcpSocket->flush();
}

#include <filehelper.h>

void RemoteControl::sendTrackInfos(const QString &track)
{
	if (!_tcpSocket) {
		return;
	}

	SqlDatabase db;

	// Send track info
	FileHelper fh(track);
	fh.artistAlbum();
	fh.album();
	fh.title();
	QString trackInfo;
	trackInfo.append("artistAlbum:%1:");

	QByteArray block;
	QDataStream out(&block, QIODevice::ReadWrite);
	out.setVersion(QDataStream::Qt_5_7);
	out << CMD_Track;
	out << db.selectTrackByURI(track);
	auto r = _tcpSocket->write(block);
	_tcpSocket->flush();
	qDebug() << Q_FUNC_INFO << "sizeof(CMD):" << sizeof(Command) << "cmd:track, bytes written" << r;

	// Send cover if any
	Cover *cover = db.selectCoverFromURI(track);
	if (cover) {
		QByteArray block;
		QDataStream out(&block, QIODevice::ReadWrite);
		out.setVersion(QDataStream::Qt_5_7);
		out << CMD_Cover;
		QByteArray c(cover->byteArray());
		out << c;
		auto r = _tcpSocket->write(block);
		_tcpSocket->flush();
		qDebug() << Q_FUNC_INFO << "cmd:cover" << c.size() << ", bytes written" << r;
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
	ba.append(reinterpret_cast<const char*>(&volume), sizeof(volume));
	out << ba;
	auto r = _tcpSocket->write(data);
	_tcpSocket->flush();
	qDebug() << Q_FUNC_INFO << "sizeof(CMD):" << sizeof(Command) << "cmd:volume, bytes written" << r;
}
