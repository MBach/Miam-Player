#include "playlistmanager.h"

#include <model/playlistdao.h>
#include <model/sqldatabase.h>
#include <settingsprivate.h>
#include "playlist.h"
#include "tabplaylist.h"

PlaylistManager::PlaylistManager(TabPlaylist *parent)
	: QObject(parent), _tabPlaylists(parent)
{
}

void PlaylistManager::saveAndRemovePlaylist(Playlist *p, int index, bool isOverwriting, bool isExiting)
{
	if (this->savePlaylist(p, isOverwriting, isExiting)) {
		emit aboutToRemovePlaylist(index);
	}
}

int PlaylistManager::savePlaylist(Playlist *p, bool isOverwriting, bool isExiting)
{
	int id = -1;
	auto _db = SqlDatabase::instance();

	int index = -1;
	for (int i = 0; i < _tabPlaylists->count(); i++) {
		Playlist *pl = _tabPlaylists->playlist(i);
		if (pl == p) {
			index = i;
			break;
		}
	}
	qDebug() << Q_FUNC_INFO << index << isOverwriting << isExiting;

	if (p && !p->mediaPlaylist()->isEmpty()) {
		QString playlistName = _tabPlaylists->tabBar()->tabText(index);

		uint generateNewHash = p->generateNewHash();

		// Check first if one has the same playlist in database
		PlaylistDAO playlist;
		for (PlaylistDAO dao : _db->selectPlaylists()) {
			if (dao.checksum().toUInt() == generateNewHash) {
				// When exiting, don't show this Dialog and just quit!
				if (isOverwriting && isExiting) {
					qDebug() << Q_FUNC_INFO << "exiting!";
					return 1;
				}
				// Playlist exists in database and user is not exiting application -> showing a popup
				QMessageBox mb;
				mb.setIcon(QMessageBox::Information);
				mb.setText(tr("There is exactly the same playlist in the Playlist Manager (known as '%1'), "\
							  "therefore it's not possible to add it twice.").arg(dao.title()));
				mb.addButton(QMessageBox::Cancel);
				mb.addButton(QMessageBox::Discard);
				mb.setDefaultButton(QMessageBox::Cancel);

				int ret = mb.exec();
				switch (ret) {
				case QMessageBox::Cancel:
					if (isExiting) {
						return 1;
					} else {
						return 0;
					}
				case QMessageBox::Discard:
					return 1;
				}
				break;
			//} else if (isOverwriting && p->hash() == dao.checksum().toUInt()) {
			} else if (isOverwriting && p->isModified()) {
				playlist = dao;
				break;
			}
		}

		// Playlist wasn't found so we cannot overwrite it
		if (playlist.checksum().isEmpty()) {
			isOverwriting = false;
		}
		playlist.setTitle(playlistName);
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
		p->setHash(generateNewHash);
	}
	return id;
}
