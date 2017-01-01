#include "playlistmanager.h"

#include <model/playlistdao.h>
#include <model/sqldatabase.h>
#include <settingsprivate.h>
#include "playlist.h"
#include "tabplaylist.h"
#include <QXmlStreamReader>

PlaylistManager::PlaylistManager(TabPlaylist *parent)
	: QObject(parent)
	, _tabPlaylists(parent)
{}

bool PlaylistManager::loadPlaylist(Playlist *p, const QFileInfo &fileInfo)
{
	QList<QMediaContent> tracks;

	QFile file(fileInfo.absoluteFilePath());
	file.open(QFile::ReadOnly);

	// M3U or M3U8 file format
	if (fileInfo.suffix().startsWith("m3u")) {
		QTextStream in(&file);
		while (!in.atEnd()) {
			QString line = in.readLine();
			tracks << QMediaContent(QUrl(line));
		}
		if (!tracks.isEmpty()) {
			p->mediaPlaylist()->setTitle(fileInfo.baseName());
		}
	} else {
		// XSPF file format
		QXmlStreamReader xml(file.readAll());
		QString title;
		while (!xml.atEnd() && !xml.hasError()) {
			xml.readNext();
			if (xml.tokenType() == QXmlStreamReader::StartElement) {
				if (xml.name() == "title" && title.isEmpty()) {
					title = xml.readElementText();
				} else if (xml.name() == "location") {
					tracks << QMediaContent(QUrl(xml.readElementText()));
				}
			}
		}
		if (!tracks.isEmpty()) {
			if (title.isEmpty()) {
				p->mediaPlaylist()->setTitle(fileInfo.baseName());
			} else {
				p->mediaPlaylist()->setTitle(title);
			}
		}
	}
	file.close();

	p->insertMedias(-1, tracks);
	return !tracks.isEmpty();
}

bool PlaylistManager::deletePlaylist(uint playlistId)
{
	return SqlDatabase().removePlaylist(playlistId);
}

uint PlaylistManager::savePlaylist(Playlist *p, bool isOverwriting, bool isExiting)
{
	uint id = 0;
	SqlDatabase db;

	for (int i = 0; i < _tabPlaylists->count(); i++) {
		Playlist *pl = _tabPlaylists->playlist(i);
		if (pl == p) {
			break;
		}
	}

	if (p && !p->mediaPlaylist()->isEmpty()) {

		uint generateNewHash = p->generateNewHash();
		PlaylistDAO playlist;

		// Check first if one has the same playlist in database
		for (PlaylistDAO dao : db.selectPlaylists()) {
			if (dao.checksum().toUInt() == generateNewHash) {
				playlist = dao;
				break;
			}
		}

		// No playlists with this checksum were found -> it's possible to write/overwrite this one
		if (playlist.id().isEmpty()) {
			// Playlist object
			if (p->id() > 0) {
				playlist.setId(QString::number(p->id()));
			}
		} else {
			if (isExiting) {
				// When exiting, don't show a Dialog and just quit!
				if (!isOverwriting) {
					return playlist.id().toUInt();
				}
			} else {
				// Playlist exists in database and user is not exiting application -> showing a popup
				QMessageBox mb;
				mb.setIcon(QMessageBox::Information);
				mb.setText(tr("There is exactly the same playlist in the Playlist Manager (known as '%1'), "\
							  "therefore it's not possible to add it twice.").arg(playlist.title()));
				mb.addButton(QMessageBox::Cancel);
				mb.addButton(QMessageBox::Discard);
				mb.setDefaultButton(QMessageBox::Cancel);

				int ret = mb.exec();
				switch (ret) {
				case QMessageBox::Cancel:
					return 0;
				case QMessageBox::Discard:
					return 1;
				}
			}
		}
		playlist.setTitle(p->mediaPlaylist()->title());
		playlist.setChecksum(QString::number(generateNewHash));

		QStringList tracks;
		for (int j = 0; j < p->mediaPlaylist()->mediaCount(); j++) {
			// Eeach track has been saved in a hidden column into the playlist
			tracks << p->model()->index(j, p->COL_TRACK_DAO).data().toString();
		}

		id = db.insertIntoTablePlaylists(playlist, tracks, isOverwriting);

		p->setId(id);
		p->setHash(generateNewHash);
	}
	return id;
}

void PlaylistManager::saveAndRemovePlaylist(Playlist *p, int index, bool isOverwriting)
{
	if (this->savePlaylist(p, isOverwriting, false) != 0) {
		emit aboutToRemovePlaylist(index);
	}
}
