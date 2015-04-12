#ifndef UNIQUELIBRARY_H
#define UNIQUELIBRARY_H

#include <QSet>
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
	QSet<GenericDAO*> _set;

public:
	explicit UniqueLibrary(QWidget *parent = 0);

	void init();
	void setVisible(bool visible);
};

#endif // UNIQUELIBRARY_H
