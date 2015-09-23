#ifndef CUSTOMIZEOPTIONSDIALOG_H
#define CUSTOMIZEOPTIONSDIALOG_H

#include <QDir>
#include <QDialog>
#include <QToolButton>

#include "ui_customizeoptionsdialog.h"

#include "plugininfo.h"
#include "pluginmanager.h"

/**
 * \brief		The CustomizeOptionsDialog class is a very important class. It is designed to help one to customize options of Miam-Player.
 * \details
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class CustomizeOptionsDialog : public QDialog, public Ui::CustomizeOptionsDialog
{
	Q_OBJECT
public:
	explicit CustomizeOptionsDialog(PluginManager *pluginManager, QWidget *parent = 0);

	/** Third panel in this dialog: shorcuts has to be initialized in the end. */
	void initShortcuts();

protected:
	/** Redefined to add custom behaviour. */
	virtual void closeEvent(QCloseEvent *) override;

	/** Redefined to inspect shortcuts. */
	virtual bool eventFilter(QObject *obj, QEvent *e) override;

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
	void deleteSelectedLocation();

	/** Open a dialog for letting the user to choose a music directory. */
	void openLibraryDialog();

	void updateMusicLocations();

	/** Insert a new row in the Plugin Page in Config Dialog with basic informations for each plugin. */
	void insertRow(const PluginInfo &pluginInfo);

signals:
	void aboutToBindShortcut(const QString &objectName, const QKeySequence &keySequence);

	void defaultLocationFileExplorerHasChanged(const QDir &location);

	/** Signal sent whether the music locations have changed or not. */
	void musicLocationsHaveChanged(const QStringList &oldLocations, const QStringList &newLocations);
};

#endif // CUSTOMIZEOPTIONSDIALOG_H
