#ifndef NOFOCUSITEMDELEGATE_H
#define NOFOCUSITEMDELEGATE_H

#include <QItemDelegate>

class NoFocusItemDelegate : public QItemDelegate
{
	Q_OBJECT
public:
	NoFocusItemDelegate(QObject *parent = 0) : QItemDelegate(parent) {}

protected:
	/** Redefined as empty to remove the dotted rectangle when selecting an item. */
	inline void drawFocus(QPainter *, const QStyleOptionViewItem &, const QRect &) const {}
};

#endif // NOFOCUSITEMDELEGATE_H
