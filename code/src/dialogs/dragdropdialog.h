#ifndef DRAGDROPDIALOG_H
#define DRAGDROPDIALOG_H

#include <QDialog>
#include <QDir>
#include <QMimeData>
#include <QUrl>

#include "ui_dragdroppopup.h"

class DragDropDialog : public QDialog, public Ui::DragDropDialog
{
	Q_OBJECT
private:
	QList<QDir> _externalLocations;
	QString originalLabel;

public:
	explicit DragDropDialog(QWidget *parent = 0);

	void setMimeData(const QMimeData *mimeData);

	inline QList<QDir> externalLocations() const { return _externalLocations; }

private slots:
	void addExternalFoldersToLibrary();
	void addExternalFoldersToPlaylist();

signals:
	void aboutToAddExtFoldersToLibrary(const QList<QDir> &);
	void aboutToAddExtFoldersToPlaylist(const QList<QDir> &);

	void rememberDragDrop(QToolButton*);
};

#endif // DRAGDROPDIALOG_H
