#include "playlistmanager.h"

#include <model/playlistdao.h>
#include <model/sqldatabase.h>
#include <settingsprivate.h>
#include "playlist.h"
#include "tabplaylist.h"

PlaylistManager::PlaylistManager(TabPlaylist *parent)
	: QObject(parent), _tabPlaylists(parent)
{}

bool PlaylistManager::deletePlaylist(uint playlistId)
{
	return SqlDatabase::instance()->removePlaylist(playlistId);
}

uint PlaylistManager::savePlaylist(Playlist *p, bool isOverwriting, bool isExiting)
{
	uint id = 0;
	auto _db = SqlDatabase::instance();

	int index = -1;
	for (int i = 0; i < _tabPlaylists->count(); i++) {
		Playlist *pl = _tabPlaylists->playlist(i);
		if (pl == p) {
			index = i;
			break;
		}
	}

	if (p && !p->mediaPlaylist()->isEmpty()) {

		uint generateNewHash = p->generateNewHash();
		PlaylistDAO playlist;

		// Check first if one has the same playlist in database
		for (PlaylistDAO dao : _db->selectPlaylists()) {
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

		playlist.setTitle(p->title());
		playlist.setChecksum(QString::number(generateNewHash));

		std::list<TrackDAO> tracks;
		const QStandardItemModel *model = qobject_cast<const QStandardItemModel *>(p->model());
		for (int j = 0; j < p->mediaPlaylist()->mediaCount(); j++) {
			// Eeach track has been saved in a hidden column into the playlist
			/// FIXME
			TrackDAO t = model->index(j, p->COL_TRACK_DAO).data().value<TrackDAO>();
			tracks.push_back(std::move(t));
		}

		id = _db->insertIntoTablePlaylists(playlist, tracks, isOverwriting);

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
