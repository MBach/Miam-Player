#ifndef LIBRARYHEADER_H
#define LIBRARYHEADER_H

#include <QPushButton>

#include "miamlibrary_global.hpp"

/**
 * \brief		The LibraryHeader class is a button which can switch the LibraryTreeView from Ascending to Descending order.
 * \details		This class also reimplements the contextMenuEvent handler to be able to display a small dialog.
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
	explicit LibraryHeader(QWidget *parent = nullptr);

protected:
	/** Reimplemented to display a dialog to with 4 hierarchies available to the user. */
	virtual void contextMenuEvent(QContextMenuEvent *) override;

	virtual void leaveEvent(QEvent *event) override;

	virtual void mouseMoveEvent(QMouseEvent *event) override;

	virtual void paintEvent(QPaintEvent *) override;

public slots:
	inline void resetSortOrder() { _order = Qt::AscendingOrder; }

signals:
	/** Forward signal to upper class. */
	void aboutToChangeHierarchyOrder();

	void aboutToChangeSortOrder();
};

#endif // LIBRARYHEADER_H
