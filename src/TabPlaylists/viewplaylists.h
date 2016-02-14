#ifndef VIEWPLAYLISTS_H
#define VIEWPLAYLISTS_H

#include <abstractviewplaylists.h>
#include "dialogs/searchdialog.h"
#include "viewplaylistsmediaplayercontrol.h"

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
	explicit ViewPlaylists(MediaPlayer *mediaPlayer, QWidget *parent = nullptr);

	virtual ~ViewPlaylists();

	virtual void addToPlaylist(const QList<QUrl> &tracks) override;

	virtual QPair<QString, QObjectList> extensionPoints() const override;

	virtual void openFolder(const QString &dir) const override;

	virtual void saveCurrentPlaylists() override;

	virtual int selectedTracksInCurrentPlaylist() const override;

	virtual void setMusicSearchEngine(MusicSearchEngine *musicSearchEngine) override;

	inline virtual QSize sizeHint() const override { return QSize(1024, 768); }

	virtual bool viewProperty(Settings::ViewProperty vp) const override;

protected:
	virtual void changeEvent(QEvent *event) override;

public slots:
	/** Redefined from AbstractView. */
	virtual void initFileExplorer(const QDir &dir) override;

	/** Redefined from AbstractViewPlaylists. */
	virtual void addExtFolders(const QList<QDir> &folders) override;

	/** Redefined from AbstractViewPlaylists. */
	virtual void addPlaylist() override;

	/** Redefined from AbstractViewPlaylists. */
	virtual void moveTracksDown() override;

	/** Redefined from AbstractViewPlaylists. */
	virtual void moveTracksUp() override;

	/** Redefined from AbstractViewPlaylists. */
	virtual void openFiles() override;

	/** Redefined from AbstractViewPlaylists. */
	virtual void openFolderPopup() override;

	/** Redefined from AbstractViewPlaylists. */
	virtual void openPlaylistManager() override;

	/** Redefined from AbstractViewPlaylists. */
	virtual void removeCurrentPlaylist() override;

	/** Redefined from AbstractViewPlaylists. */
	virtual void removeSelectedTracks() override;

	virtual void setViewProperty(Settings::ViewProperty vp, QVariant value) override;

	virtual void volumeSliderDecrease() override;

	virtual void volumeSliderIncrease() override;

private slots:
	void mediaPlayerStateHasChanged(QMediaPlayer::State state);
};

#endif // VIEWPLAYLISTS_H
