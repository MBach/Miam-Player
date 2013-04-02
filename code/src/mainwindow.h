#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtMultimedia/QAudioOutput>

#include "ui_mainwindow.h"
#include "dialogs/customizeoptionsdialog.h"
#include "dialogs/dragdropdialog.h"
#include "dialogs/playlistmanager.h"
#include "library/librarytreeview.h"
#include "settings.h"
#include "mediabutton.h"

/// Need to use this forward declaration (circular inclusion)
class CustomizeThemeDialog;

class MainWindow : public QMainWindow, public Ui::MainWindow
{
	Q_OBJECT
public:
	MainWindow(QWidget *parent = 0);

	// Play, pause, stop, etc.
	QList<MediaButton*> mediaButtons;

private:
	CustomizeThemeDialog *customizeThemeDialog;
	CustomizeOptionsDialog *customizeOptionsDialog;
	PlaylistManager *playlistManager;
	DragDropDialog *dragDropDialog;

	/** Set up all actions and behaviour. */
	void setupActions();

	QAudioOutput *audioOutput;

	// MP3 actions
	QAction *actionPlay;
	QAction *actionPause;
	QAction *actionStop;

protected:
	/** Redefined to be able to retransltate User Interface at runtime. */
	void changeEvent(QEvent *event);

	void closeEvent(QCloseEvent *event);

	void dropEvent(QDropEvent *event);
	void dragEnterEvent(QDragEnterEvent *event);
	void dragMoveEvent(QDragMoveEvent *event);

public slots:
	void bindShortcut(const QString&, int keySequence);

private slots:
	void drawLibrary(bool b=false);

	/** Change the labels like "Remove selected track(s)" depending of the number of selected elements in the current playlist. */
	void changeMenuLabels(int);

	/// Media actions
	/** These 2 buttons toggle play and pause functions because they are mutually exclusive. */
	void playAndPause();

	/** If playing, then stops the track. */
	void stop();

	/** Displays a simple message box about MmeMiamMiamMusicPlayer. */
	void aboutM4P();

	void toggleTagEditor(bool b);
};

#endif // MAINWINDOW_H
