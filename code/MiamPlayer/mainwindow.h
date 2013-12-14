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

/// Need to use this forward declaration (circular inclusion)
class CustomizeThemeDialog;

class MainWindow : public QMainWindow, public Ui::MainWindow
{
	Q_OBJECT
public:
	// Play, pause, stop, etc.
	QList<MediaButton*> mediaButtons;

	MainWindow(QWidget *parent = 0);

	void init();

	void loadPlugins();

	/** Update fonts for menu and context menus. */
	void updateFonts(const QFont &font);

private:
	CustomizeThemeDialog *customizeThemeDialog;
	CustomizeOptionsDialog *customizeOptionsDialog;
	PlaylistManager *playlistManager;
	DragDropDialog *dragDropDialog;
	PlaybackModeWidgetFactory *playbackModeWidgetFactory;
	QSharedPointer<MediaPlayer> _mediaPlayer;
	UniqueLibrary *_uniqueLibrary;
	LibrarySqlModel *_librarySqlModel;

	/** Set up all actions and behaviour. */
	void setupActions();

	QAudioOutput *audioOutput;

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
	//void drawLibrary(bool b = false);

	/** Displays a simple message box about MmeMiamMiamMusicPlayer. */
	void aboutM4P();

	void toggleTagEditor(bool b);
};

#endif // MAINWINDOW_H
