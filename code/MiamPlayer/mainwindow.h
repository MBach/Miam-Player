#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtMultimedia/QAudioOutput>

#include "ui_mainwindow.h"
#include "dialogs/customizeoptionsdialog.h"
#include "dialogs/dragdropdialog.h"
#include "dialogs/playlistmanager.h"
#include "library/librarytreeview.h"
#include "mediabutton.h"
#include <mediaplayer.h>
#include "playbackmodewidgetfactory.h"

#include <model/librarysqlmodel.h>

#include "uniquelibrary.h"
#include "sqldatabase.h"

class CustomizeThemeDialog;

class MainWindow : public QMainWindow, public Ui::MainWindow
{
	Q_OBJECT
private:
	CustomizeThemeDialog *customizeThemeDialog;
	PlaylistManager *playlistManager;
	DragDropDialog *dragDropDialog;
	PlaybackModeWidgetFactory *playbackModeWidgetFactory;
	QSharedPointer<MediaPlayer> _mediaPlayer;
	UniqueLibrary *_uniqueLibrary;
	LibrarySqlModel *_librarySqlModel;
	QActionGroup *_viewModeGroup;
	QAudioOutput *audioOutput;
	SqlDatabase *_sqlDatabase;

public:
	// Play, pause, stop, etc.
	QList<MediaButton*> mediaButtons;
	CustomizeOptionsDialog *customizeOptionsDialog;

	MainWindow(QWidget *parent = 0);

	virtual ~MainWindow();

	void init();

	/** Plugins. */
	void loadPlugins();

	/** Update fonts for menu and context menus. */
	void updateFonts(const QFont &font);

	void loadPlugin(const QString &pluginAbsoluteFilePath);

	/** Set up all actions and behaviour. */
	void setupActions();

	QWeakPointer<MediaPlayer> mediaPlayer() const { return _mediaPlayer; }

protected:
	/** Redefined to be able to retransltate User Interface at runtime. */
	void changeEvent(QEvent *event);

	void closeEvent(QCloseEvent *event);

	void dropEvent(QDropEvent *event);
	void dragEnterEvent(QDragEnterEvent *event);
	void dragMoveEvent(QDragMoveEvent *event);

	void moveEvent(QMoveEvent *);

public slots:
	void bindShortcut(const QString&, int keySequence);

private slots:
	/** Displays a simple message box about MmeMiamMiamMusicPlayer. */
	void aboutM4P();

	void toggleTagEditor(bool b);
};

#endif // MAINWINDOW_H
