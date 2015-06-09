#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCommandLineParser>
#include <QMainWindow>
#include <QStack>

#include <model/sqldatabase.h>
#include <mediabutton.h>
#include <mediaplayer.h>
#include "dialogs/customizeoptionsdialog.h"
#include "dialogs/playlistdialog.h"
#include "library/librarytreeview.h"
#include "playbackmodewidgetfactory.h"
#include "searchdialog.h"

#include "uniquelibrary.h"
#include "ui_mainwindow.h"

/**
 * \brief The MainWindow class is the entry point of this audio player.
 */
class MainWindow : public QMainWindow, public Ui::MainWindow
{
	Q_OBJECT
private:
	/** Displays and animates the media button "PlaybackMode". */
	PlaybackModeWidgetFactory *playbackModeWidgetFactory;

	/** WIP. View object: display all your tracks in a huge and page. */
	UniqueLibrary *_uniqueLibrary;

	SearchDialog *_searchDialog;

public:
	// Play, pause, stop, etc.
	QList<MediaButton*> mediaButtons;
	CustomizeOptionsDialog *customizeOptionsDialog;

	MainWindow(QWidget *parent = 0);

	void activateLastView();

	void dispatchDrop(QDropEvent *event);

	void init();

	/** Plugins. */
	void loadPlugins();

	void moveSearchDialog();

	inline AbstractSearchDialog * searchDialog() const { return _searchDialog; }

	/** Set up all actions and behaviour. */
	void setupActions();

	/** Update fonts for menu and context menus. */
	void updateFonts(const QFont &font);

	static QMessageBox::StandardButton showWarning(const QString &target, int count);

	void openFolder(const QString &dir);

protected:
	/** Redefined to be able to retransltate User Interface at runtime. */
	virtual void changeEvent(QEvent *event) override;

	virtual void closeEvent(QCloseEvent *) override;

	virtual void dragEnterEvent(QDragEnterEvent *event) override;

	virtual void dragMoveEvent(QDragMoveEvent *event) override;

	virtual void dropEvent(QDropEvent *event) override;

	virtual bool event(QEvent *event) override;

	virtual void moveEvent(QMoveEvent *event) override;

private:
	void loadTheme();

public slots:
	void processArgs(const QStringList &args);

private slots:
	void bindShortcut(const QString&, const QKeySequence &keySequence);

	void mediaPlayerStateHasChanged(QMediaPlayer::State state);

	void openFiles();

	void openFolderPopup();

	void openPlaylistManager();

	void showTabPlaylists();

	void showTagEditor();
};

#endif // MAINWINDOW_H
