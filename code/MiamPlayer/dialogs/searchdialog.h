#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QDialog>
#include "ui_searchdialog.h"

class SearchDialog : public QDialog, public Ui::SearchDialog
{
	Q_OBJECT
public:
	explicit SearchDialog(QWidget *parent = 0);
	
signals:
	
public slots:
	
};

#endif // SEARCHDIALOG_H
