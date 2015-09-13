#ifndef MIAMITEMDELEGATE_H
#define MIAMITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QSortFilterProxyModel>
#include <QTimer>
#include "libraryitemmodel.h"
#include "trackitem.h"

#include "miamlibrary_global.h"

class MIAMLIBRARY_LIBRARY MiamItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT
protected:
	static qreal _iconOpacity;

	QStandardItemModel *_libraryModel;
	QSortFilterProxyModel *_proxy;
	bool _showCovers;

	/** This timer is used to animate album cover when one is scrolling.
	 * It improves reactivity of the UI by temporarily disabling painting events.
	 * When covers are becoming visible once again, they are redisplayed with a nice fading effect. */
	QTimer *_timer;

	int _coverSize;

public:
	MiamItemDelegate(QSortFilterProxyModel *proxy);

protected:
	void drawLetter(QPainter *painter, QStyleOptionViewItem &option, SeparatorItem *item) const;

	virtual void drawTrack(QPainter *painter, QStyleOptionViewItem &option, TrackItem *track) const;

	void paintRect(QPainter *painter, const QStyleOptionViewItem &option) const;

	void paintText(QPainter *p, const QStyleOptionViewItem &opt, const QRect &rectText, const QString &text, const QStandardItem *item) const;
};

#endif // MIAMITEMDELEGATE_H
