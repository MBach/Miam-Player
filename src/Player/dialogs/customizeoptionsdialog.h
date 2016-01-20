#ifndef CUSTOMIZEOPTIONSDIALOG_H
#define CUSTOMIZEOPTIONSDIALOG_H

#include <QDir>
#include <QDialog>
#include <QToolButton>

#include "ui_customizeoptionsdialog.h"

#include "pluginmanager.h"

/**
 * \brief		The CustomizeOptionsDialog class is a very important class. It is designed to help one to customize options of Miam-Player.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class CustomizeOptionsDialog : public QDialog, public Ui::CustomizeOptionsDialog
{
	Q_OBJECT
private:
	PluginManager *_pluginManager;

public:
	explicit CustomizeOptionsDialog(PluginManager *pluginManager, QWidget *parent = nullptr);

	/** Redefined to inspect shortcuts. */
	virtual bool eventFilter(QObject *obj, QEvent *e) override;

protected:
	/** Redefined to add custom behaviour. */
	virtual void closeEvent(QCloseEvent *) override;

private:
	/** Sixth and last panel: list all plugins (enabled of not). */
	void initPlugins();

	/** Third panel of this dialog: shorcuts. */
	void initShortcuts();

public slots:
	/** Adds a new music location in the library. */
	void addMusicLocation(const QDir &musicLocation);

	/** Adds a external music locations in the library (Drag & Drop). */
	void addMusicLocations(const QList<QDir> &dirs);

private slots:
	/** Application can be retranslated dynamically at runtime. */
	void changeLanguage();

	/** Verify that one hasn't tried to bind a key twice. */
	void checkShortcutsIntegrity();

	/** Delete a music location previously chosen by the user. */
	void deleteMusicLocation();

	/** Open a dialog for letting the user to choose a music directory. */
	void openLibraryDialog();

	/** Load or unload a plugin when one is switching a checkbox in the options. */
	void togglePlugin(QTableWidgetItem *item);

	/** Check if music locations have changed in order to rescan the filesystem. */
	void updateMusicLocations();

signals:
	void aboutToBindShortcut(const QString &objectName, const QKeySequence &keySequence);

	void defaultLocationFileExplorerHasChanged(const QDir &location);
};

#endif // CUSTOMIZEOPTIONSDIALOG_H
