#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <phonon>

#include "ui_mainwindow.h"
#include "dialogs/customizeoptionsdialog.h"
#include "dialogs/playlistmanager.h"
#include "library/librarytreeview.h"
#include "settings.h"
#include "mediabutton.h"

/// Need to use this forward declaration (circular inclusion)
class CustomizeThemeDialog;


using namespace Phonon;

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

	/** Set up all actions and behaviour. */
	void setupActions();

	AudioOutput *audioOutput;

	// MP3 actions
	QAction *actionPlay;
	QAction *actionPause;
	QAction *actionStop;

protected:
	/** Redefined to be able to retransltate User Interface at runtime. */
	void changeEvent(QEvent *event);

	void closeEvent(QCloseEvent *event);

public slots:
	void bindShortcut(const QString&, int keySequence);

private slots:
	void drawLibrary(bool b=false);

	/** Change the labels like "Remove selected track(s)" depending of the number of selected elements in the current playlist. */
	void changeMenuLabels(int);

	/// Media actions
	/** These buttons switch the play function with the pause function because they are mutually exclusive. */
	void playAndPause();

	/** If playing, then stops the track. */
	void stop();

	/** Displays a simple message box about MmeMiamMiamMusicPlayer. */
	void aboutM4P();

	void toggleTagEditor(bool b);
};

#endif // MAINWINDOW_H
