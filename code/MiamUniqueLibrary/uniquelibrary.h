#ifndef UNIQUELIBRARY_H
#define UNIQUELIBRARY_H

#include <QSet>
#include <QWidget>

#include "miamuniquelibrary_global.h"
#include "model/sqldatabase.h"

#include "ui_uniquelibrary.h"

/**
* \brief
* \details
* \author      Matthieu Bachelier
* \copyright   GNU General Public License v3
* */
class MIAMUNIQUELIBRARY_LIBRARY UniqueLibrary : public QWidget, public Ui::UniqueLibrary
{
	Q_OBJECT

private:
	QSet<GenericDAO*> _set;

public:
	explicit UniqueLibrary(QWidget *parent = 0);
};

#endif // UNIQUELIBRARY_H
