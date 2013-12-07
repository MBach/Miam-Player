#ifndef LIBRARYORDERDIALOG_H
#define LIBRARYORDERDIALOG_H

#include <QDialog>

#include "ui_libraryorderdialog.h"

//#include <model/librarymodel.h>

class LibraryOrderDialog : public QDialog, public Ui::LibraryOrderDialog
{
	Q_OBJECT

//private:
//	LibraryModel *_model;
	
public:
	explicit LibraryOrderDialog(QWidget *parent = 0);

//	void setModel(LibraryModel *model) { _model = model; }

public slots:
	void show();

signals:
	void aboutToRedrawLibrary(bool);
};

#endif // LIBRARYORDERDIALOG_H
