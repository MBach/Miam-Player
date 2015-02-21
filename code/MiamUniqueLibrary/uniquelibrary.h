#ifndef UNIQUELIBRARY_H
#define UNIQUELIBRARY_H

#include <QSet>
#include <QStandardItemModel>
#include <QWidget>

#include "miamuniquelibrary_global.h"
#include "model/sqldatabase.h"

namespace Ui {
class UniqueLibrary;
}

class MIAMUNIQUELIBRARY_LIBRARY UniqueLibrary : public QWidget
{
	Q_OBJECT

private:
	Ui::UniqueLibrary *ui;
	SqlDatabase *_db;
	QStandardItemModel *_model;
	QSet<GenericDAO*> _set;

public:
	explicit UniqueLibrary(QWidget *parent = 0);

	void init(SqlDatabase *db);
	void setVisible(bool visible);
	void insertNode(GenericDAO *node);
	void updateNode(GenericDAO *node);

private slots:
	void reset();
};

#endif // UNIQUELIBRARY_H
