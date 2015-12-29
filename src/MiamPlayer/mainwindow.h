#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCommandLineParser>
#include <QMainWindow>
#include <QStack>

#include <model/sqldatabase.h>
#include <librarytreeview.h>
#include <mediabutton.h>
#include <mediaplayer.h>
#include <uniquelibrary.h>

#include "dialogs/customizeoptionsdialog.h"
#include "dialogs/playlistdialog.h"
#include "dialogs/searchdialog.h"
#include "playbackmodewidgetfactory.h"
#include "pluginmanager.h"

#include "ui_mainwindow.h"

/**
 * \brief		The MainWindow class is the entry point of this audio player.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MainWindow : public QMainWindow, public Ui::MainWindow
{
	Q_OBJECT
private:
	/** Displays and animates the media button "PlaybackMode". */
	PlaybackModeWidgetFactory *_playbackModeWidgetFactory;

	MediaPlayer *_mediaPlayer;

	/** View object: display all your tracks in a huge list. */
	UniqueLibrary *_uniqueLibrary;

	PluginManager *_pluginManager;

public:
	// Play, pause, stop, etc.
	QList<MediaButton*> mediaButtons;
	SearchDialog *searchDialog;

	explicit MainWindow(QWidget *parent = nullptr);

	void activateLastView();

	void dispatchDrop(QDropEvent *event);

	virtual bool eventFilter(QObject *watched, QEvent *event) override;

	void init();

	/** Plugins. */
	void loadPlugins();

	inline MediaPlayer *mediaPlayer() const { return _mediaPlayer; }

	/** Open a new Dialog where one can add a folder to current playlist. */
	void openFolder(const QString &dir);

	/** Set up all actions and behaviour. */
	void setupActions();

	/** Update fonts for menu and context menus. */
	void updateFonts(const QFont &font);

protected:
	/** Redefined to be able to retransltate User Interface at runtime. */
	virtual void changeEvent(QEvent *event) override;

	virtual void closeEvent(QCloseEvent *) override;

	virtual void dragEnterEvent(QDragEnterEvent *event) override;

	virtual void dragMoveEvent(QDragMoveEvent *event) override;

	virtual void dropEvent(QDropEvent *event) override;

	virtual bool event(QEvent *event) override;

	virtual void moveEvent(QMoveEvent *event) override;

	virtual void resizeEvent(QResizeEvent *e) override;

private:
	void loadThemeAndSettings();

public slots:
	void createCustomizeOptionsDialog();

	void processArgs(const QStringList &args);

private slots:
	void bindShortcut(const QString&, const QKeySequence &keySequence);

	void mediaPlayerStateHasChanged(QMediaPlayer::State state);

	void openFiles();

	void openFolderPopup();

	void openPlaylistManager();

	void showTabPlaylists();

	void showTagEditor();

	void toggleMenuBar(bool checked);
};

#endif // MAINWINDOW_H
