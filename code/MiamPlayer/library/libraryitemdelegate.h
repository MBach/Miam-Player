#ifndef LIBRARYITEMDELEGATE_H
#define LIBRARYITEMDELEGATE_H

#include <QStyledItemDelegate>

#include <QPainter>

#include "library/libraryfilterproxymodel.h"
#include "albumitem.h"
#include "artistitem.h"
#include "discitem.h"
#include "separatoritem.h"
#include "trackitem.h"
#include "yearitem.h"

#include <QPointer>
#include <QPropertyAnimation>
#include <QStandardItem>

class LibraryTreeView;

class LibraryItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT
private:
	QPointer<LibraryFilterProxyModel> _proxy;

	QPointer<QStandardItemModel> _libraryModel;

	bool _showCovers;
	bool _animateIcons;

	qreal _iconOpacity;

	LibraryTreeView *_libraryTreeView;

public:
	LibraryItemDelegate(LibraryTreeView *libraryTreeView, LibraryFilterProxyModel *proxy);

	virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

	/** Redefined to always display the same height for albums, even for those without one. */
	virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

	inline void setIconOpacity(qreal opacity) {
		_iconOpacity = opacity;
		if (_iconOpacity == 1.0) {
			_animateIcons = false;
		}
	}

private:
	/** Albums have covers usually. */
	void drawAlbum(QPainter *painter, QStyleOptionViewItem &option, AlbumItem *item) const;

	void drawArtist(QPainter *painter, QStyleOptionViewItem &option, ArtistItem *item) const;

	void drawDisc(QPainter *painter, QStyleOptionViewItem &option, DiscItem *item) const;

	void drawLetter(QPainter *painter, QStyleOptionViewItem &option, SeparatorItem *item) const;

	void drawTrack(QPainter *painter, QStyleOptionViewItem &option, const QStandardItem *track) const;

	void paintCoverOnTrack(QPainter *painter, const QStyleOptionViewItem &option, const TrackItem *track) const;
	void paintRect(QPainter *painter, const QStyleOptionViewItem &option) const;

	/** Check if color needs to be inverted then paint text. */
	void paintText(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rectText, const QString &text, const QStandardItem *item) const;

public slots:
	void displayIcon(bool b);
};

#endif // LIBRARYITEMDELEGATE_H
