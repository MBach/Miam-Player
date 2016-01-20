#ifndef NOFOCUSITEMDELEGATE_H
#define NOFOCUSITEMDELEGATE_H

#include <QItemDelegate>
#include <QPainter>

/**
 * \brief		The NoFocusItemDelegate class cancel drawFocus method.
 * \details		This class does only one thing: ignore the "focus dotted rectangle" when a cell
 *				(or a row) is selected in a view.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class NoFocusItemDelegate : public QItemDelegate
{
	Q_OBJECT
public:
	NoFocusItemDelegate(QWidget *parent = nullptr) : QItemDelegate(parent) {}

protected:
	/** Redefined as empty to remove the dotted rectangle when selecting an item. */
	inline void drawFocus(QPainter *, const QStyleOptionViewItem &, const QRect &) const {}
};

#endif // NOFOCUSITEMDELEGATE_H
