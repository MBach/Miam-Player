#ifndef VIEWPLAYLISTS_H
#define VIEWPLAYLISTS_H

#include <abstractviewplaylists.h>
#include "dialogs/searchdialog.h"

#include "ui_viewplaylists.h"

/**
 * \brief		The ViewPlaylists class is the implementation of AbstractViewPlaylists.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMTABPLAYLISTS_LIBRARY ViewPlaylists : public AbstractViewPlaylists, public Ui::ViewPlaylists
{
	Q_OBJECT
private:
	SearchDialog *_searchDialog;
	QTranslator translator;
	SqlDatabase *_db;

public:
	ViewPlaylists(MediaPlayer *mediaPlayer);

	virtual ~ViewPlaylists();

	virtual void addToPlaylist(const QList<QUrl> &tracks) override;

	virtual void openFolder(const QString &dir) const override;

	virtual void saveCurrentPlaylists() override;

	virtual int selectedTracksInCurrentPlaylist() const override;

	virtual void setDatabase(SqlDatabase *db) override;

	virtual bool viewProperty(SettingsPrivate::ViewProperty vp) const override;

protected:
	virtual void changeEvent(QEvent *event) override;

public slots:
	virtual void addExtFolders(const QList<QDir> &folders) override;

	virtual void addPlaylist() override;

	virtual void initFileExplorer(const QDir &dir) override;

	virtual void moveTracksDown() override;

	virtual void moveTracksUp() override;

	virtual void openFiles() override;

	virtual void openFolderPopup() override;

	virtual void openPlaylistManager() override;

	virtual void removeCurrentPlaylist() override;

	virtual void removeSelectedTracks() override;

	virtual void setViewProperty(SettingsPrivate::ViewProperty vp, QVariant value) override;

	virtual void volumeSliderDecrease() override;

	virtual void volumeSliderIncrease() override;

private slots:
	void mediaPlayerStateHasChanged(QMediaPlayer::State state);
};

#endif // VIEWPLAYLISTS_H
