#ifndef DRAGDROPDIALOG_H
#define DRAGDROPDIALOG_H

#include <QDialog>
#include <QDir>
#include <QMimeData>
#include <QUrl>

#include "ui_dragdroppopup.h"

/**
 * \brief		The DragDropDialog class is a small modal dialog.
 * \details		This dialog is displayed on screen when one is interacting from an external application to the Miam-Player.
 *	One can drag files or folders from any filesystem explorer to this application. The first time this dialog will appear,
 *	2 options will be proposed: to add items to the current playlist, or to add them to the library. This choice can be remembered.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class DragDropDialog : public QDialog, public Ui::DragDropDialog
{
	Q_OBJECT
private:
	QList<QDir> _externalLocations;
	QString originalLabel;

public:
	explicit DragDropDialog(QWidget *parent = 0);

	void retranslateUi(DragDropDialog *dialog);

	bool setMimeData(const QMimeData *mimeData);

	inline const QList<QDir> & externalLocations() const { return _externalLocations; }

private slots:
	void addExternalFoldersToLibrary();
	void addExternalFoldersToPlaylist();

signals:
	void aboutToAddExtFoldersToLibrary(const QList<QDir> &);
	void aboutToAddExtFoldersToPlaylist(const QList<QDir> &);
};

#endif // DRAGDROPDIALOG_H
