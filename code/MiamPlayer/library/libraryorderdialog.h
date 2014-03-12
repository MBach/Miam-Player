#ifndef LIBRARYORDERDIALOG_H
#define LIBRARYORDERDIALOG_H

#include <QDialog>

#include "ui_libraryorderdialog.h"

class LibraryOrderDialog : public QDialog, public Ui::LibraryOrderDialog
{
	Q_OBJECT
	
public:
	explicit LibraryOrderDialog(QWidget *parent = 0);

protected:
	virtual void paintEvent(QPaintEvent *);

public slots:
	void show();

signals:
	void aboutToRedrawLibrary();
};

#endif // LIBRARYORDERDIALOG_H
