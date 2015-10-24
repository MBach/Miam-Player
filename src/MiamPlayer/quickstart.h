#ifndef QUICKSTARTTABLEWIDGET_H
#define QUICKSTARTTABLEWIDGET_H

#include <QFileInfo>
#include <QWidget>

#include <quickstartsearchengine.h>

#include "ui_quickstart.h"

/// Forward declaration
class MainWindow;

/**
 * \brief		The QuickStart class is used at startup to display a list of places where one can choose how to scan his harddrive.
 * \details		This class extends QWidget and offers to the user 3 differents ways to add filesystem path.
 *		-#	On recent systems, there is a default music folder that can be selected.
 *		-#	This second choice is the default music folder but displayed in details. Each subdirectory is listed and one can
 *			manually select folders. Empty folders are unchecked by default.
 *		-#	The last one is just a QCommandLinkButton, if one wants to browse the filesystem. It actually opens the CustomizeOptionsDialog
 *			and it's possible to be very precise in how path will be added to the Library.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class QuickStart : public QWidget, public Ui::QuickStart
{
	Q_OBJECT
private:
	static const QList<int> ratios;

	int _totalMusicFiles;

	QThread *_worker;
	QuickStartSearchEngine *_qsse;

public:
	explicit QuickStart(MainWindow *mainWindow);

	void searchMultimediaFiles();

	virtual bool eventFilter(QObject *, QEvent *e) override;

protected:
	virtual void paintEvent(QPaintEvent *) override;

private:
	void applyButtonClicked(MainWindow *mainWindow, const QStringList &newLocations);

	/** The first time the player is launched, this function will scan for multimedia files. */

private slots:
	/** Check or uncheck rows when one is clicking, but not only on the checkbox. */
	void checkRow(QTableWidgetItem *i);

public slots:
	/** Insert above other rows a new one with a Master checkbox to select/unselect all. */
	void insertFirstRow();

	/** Insert a row with a checkbox with folder's name and the number of files in this folder. */
	void insertRow(const QFileInfo &, const int &);
};

#endif // QUICKSTARTTABLEWIDGET_H
