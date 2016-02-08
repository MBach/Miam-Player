#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCommandLineParser>
#include <QMainWindow>
#include <QStack>

#include <abstractview.h>
#include <mediaplayer.h>
#include <uniquelibrary.h>

#include "dialogs/customizeoptionsdialog.h"
#include "dialogs/playlistdialog.h"
#include "pluginmanager.h"
#include "views/tageditor/tageditor.h"

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
	MediaPlayer *_mediaPlayer;

	PluginManager *_pluginManager;

	AbstractView *_currentView;
	TagEditor *_tagEditor;
	QTranslator _translator;

public:
	explicit MainWindow(QWidget *parent = nullptr);

	void activateLastView();

	void dispatchDrop(QDropEvent *event);

	virtual bool eventFilter(QObject *watched, QEvent *event) override;

	void init();

	void loadPlugins();

	inline MediaPlayer *mediaPlayer() const { return _mediaPlayer; }

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

private:
	void initQuickStart();

public slots:
	void createCustomizeOptionsDialog();

	void processArgs(const QStringList &args);

private slots:
	void activateView(QAction *menuAction);

	void bindShortcut(const QString&, const QKeySequence &keySequence);

	void rescanLibrary();

	void showTagEditor();

	void toggleMenuBar(bool checked);
};

#endif // MAINWINDOW_H
