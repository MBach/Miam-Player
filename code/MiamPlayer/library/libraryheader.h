#ifndef LIBRARYHEADER_H
#define LIBRARYHEADER_H

#include <QPushButton>

#include "libraryorderdialog.h"

class LibraryHeader : public QPushButton
{
	Q_OBJECT
private:
	LibraryOrderDialog *_lod;

	Qt::SortOrder _order;

public:
	explicit LibraryHeader(QWidget *parent = 0);

protected:
	virtual void contextMenuEvent(QContextMenuEvent *);

	virtual void paintEvent(QPaintEvent *);

signals:
	void aboutToChangeHierarchyOrder();
	void aboutToChangeSortOrder();
};

#endif // LIBRARYHEADER_H
