#ifndef LIBRARYFILTERLINEEDIT_H
#define LIBRARYFILTERLINEEDIT_H

#include <QLineEdit>

#include "miamcore_global.h"

class MIAMCORE_LIBRARY LibraryFilterLineEdit : public QLineEdit
{
	Q_OBJECT

public:
	LibraryFilterLineEdit(QWidget *parent = 0);
};

#endif // LIBRARYFILTERLINEEDIT_H
