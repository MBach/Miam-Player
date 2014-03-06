#ifndef LIBRARYITEMDELEGATE_H
#define LIBRARYITEMDELEGATE_H

#include <QStyledItemDelegate>

#include <QPainter>

#include "libraryfilterproxymodel.h"

#include <QPointer>
#include <QPropertyAnimation>
#include <QStandardItem>

class LibraryItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT
private:
	QPointer<LibraryFilterProxyModel> _proxy;

	QPointer<QStandardItemModel> _libraryModel;

	bool _showCovers;
	bool _animateIcons;
	bool _iconSizeChanged;

	qreal _iconOpacity;

public:
	LibraryItemDelegate(LibraryFilterProxyModel *proxy);

	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

	/** Redefined to always display the same height for albums, even for those without one. */
	QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const;

	inline void iconSizeChanged() { _iconSizeChanged = true; }

	inline void setIconOpacity(qreal opacity) {
		_iconOpacity = opacity;
		if (_iconOpacity == 1.0) {
			_animateIcons = false;
		}
	}

private:
	/** Albums have covers usually. */
	void drawAlbum(QPainter *painter, QStyleOptionViewItem &o, QStandardItem *item) const;

	void drawArtist(QPainter *painter, QStyleOptionViewItem &option) const;

	void drawDisc(QPainter *painter, QStyleOptionViewItem &option, const QModelIndex &index) const;

	void drawLetter(QPainter *painter, QStyleOptionViewItem &option, const QModelIndex &index) const;

	void drawTrack(QPainter *painter, QStyleOptionViewItem &option, const QStandardItem *track) const;

	void paintRect(QPainter *painter, const QStyleOptionViewItem &option) const;

	/** Check if color needs to be inverted then paint text. */
	void paintText(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rectText, const QString &text) const;

public slots:
	void displayIcon(bool b);
};

#endif // LIBRARYITEMDELEGATE_H
