#ifndef LIBRARYHEADER_H
#define LIBRARYHEADER_H

#include <QPushButton>

#include "miamcore_global.h"

class MIAMCORE_LIBRARY LibraryHeader : public QPushButton
{
	Q_OBJECT
public:
	explicit LibraryHeader(QWidget *parent = 0);

protected:
	virtual void contextMenuEvent(QContextMenuEvent *);

	virtual void paintEvent(QPaintEvent *);
};

#endif // LIBRARYHEADER_H
