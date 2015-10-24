#ifndef LIBRARYHEADER_H
#define LIBRARYHEADER_H

#include <QPushButton>

#include "libraryorderdialog.h"
#include "miamlibrary_global.hpp"

/**
 * \brief		The LibraryHeader class is a button which can switch the LibraryTreeView from Ascending to Descending order.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMLIBRARY_LIBRARY LibraryHeader : public QPushButton
{
	Q_OBJECT
private:
	Qt::SortOrder _order;

	bool _uncheck;

public:
	LibraryOrderDialog *libraryOrderDialog;

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
