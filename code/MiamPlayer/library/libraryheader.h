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

	bool _uncheck;

public:
	explicit LibraryHeader(QWidget *parent = 0);

protected:
	virtual void contextMenuEvent(QContextMenuEvent *);

	virtual void paintEvent(QPaintEvent *);

	virtual bool eventFilter(QObject *obj, QEvent *event);

public slots:
	void showDialog(bool enabled);

	inline void resetSortOrder() { _order = Qt::AscendingOrder; }

signals:
	void aboutToChangeHierarchyOrder();
	void aboutToChangeSortOrder();
};

#endif // LIBRARYHEADER_H
