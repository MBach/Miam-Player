#include "playlistitemdelegate.h"

#include "playlist.h"
#include "settings.h"
#include "stareditor.h"
#include "filehelper.h"

#include <QApplication>
#include <QPainter>
#include <QStylePainter>

#include <QtDebug>

PlaylistItemDelegate::PlaylistItemDelegate(Playlist *playlist) :
	QStyledItemDelegate(playlist), _playlist(playlist)
{}

/** Redefined. */
QWidget* PlaylistItemDelegate::createEditor(QWidget *p, const QStyleOptionViewItem &, const QModelIndex &index) const
{
	StarEditor *editor = new StarEditor(index, p);
	connect(editor, &StarEditor::editingFinished, this, &PlaylistItemDelegate::commitAndClose);
	return editor;
}

void PlaylistItemDelegate::commitAndClose()
{
	// Multiple editors might have been opened by one, therefore it's required to commit and close all of them
	foreach (StarEditor *se, parent()->findChildren<StarEditor*>()) {
		QMediaContent mediaContent = _playlist->mediaPlaylist()->media(se->index().row());
		FileHelper fh(QString(QFile::encodeName(mediaContent.canonicalUrl().toLocalFile())));
		fh.setRating(se->starRating.starCount());
		commitData(se);
		closeEditor(se);
	}
}

/** Redefined. */
void PlaylistItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
	qDebug() << Q_FUNC_INFO;
	StarEditor *starEditor = qobject_cast<StarEditor *>(editor);
	qDebug() << starEditor << starEditor->starRating.starCount();
	model->setData(index, QVariant::fromValue(starEditor->starRating));
	starEditor->deleteLater();
}

/** Redefined. */
void PlaylistItemDelegate::paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
	QStyleOptionViewItem o(opt);
	initStyleOption(&o, index);
	QStyle *style = o.widget ? o.widget->style() : QApplication::style();
	o.state &= ~QStyle::State_HasFocus;
	p->save();
	p->setPen(o.palette.mid().color());
	if (QApplication::isLeftToRight() && index.column() == 0) {
		p->drawLine(o.rect.topLeft(), o.rect.bottomLeft());
	} else if (!QApplication::isLeftToRight() && index.column() == 0) {
		p->drawLine(o.rect.topRight(), o.rect.bottomRight());
	}
	Settings *settings = Settings::getInstance();
	// Light color when mouse is over
	/*if (o.state.testFlag(QStyle::State_MouseOver) && !o.state.testFlag(QStyle::State_Selected)) {
		qDebug() << "ici";
		p->setPen(opt.palette.highlight().color().darker(150));
		if (settings->isCustomColors()) {
			p->fillRect(o.rect, opt.palette.highlight().color().lighter(160));
		} else {
			p->fillRect(o.rect, opt.palette.highlight().color().lighter(170));
		}
	} else*/
	if (opt.state.testFlag(QStyle::State_Selected)) {
		p->setPen(opt.palette.highlight().color().darker(150));
		if (settings->isCustomColors()) {
			p->fillRect(o.rect, opt.palette.highlight().color());
		} else {
			p->fillRect(o.rect, opt.palette.highlight().color().lighter());
		}

		// Don't display the upper line is the track above is selected
		QModelIndex top = index.sibling(index.row() - 1, index.column());
		if (!top.isValid() || !_playlist->selectionModel()->selectedIndexes().contains(top)) {
			p->drawLine(opt.rect.topLeft(), opt.rect.topRight());
		}
		if (opt.rect.left() == 0) {
			p->drawLine(opt.rect.topLeft(), opt.rect.bottomLeft());
		} else if (opt.rect.right() == _playlist->viewport()->rect().right()) {
			p->drawLine(opt.rect.topRight(), opt.rect.bottomRight());
		}
		// Don't display the bottom line is the track underneath is selected
		QModelIndex bottom = index.sibling(index.row() + 1, index.column());
		if (!bottom.isValid() || !_playlist->selectionModel()->selectedIndexes().contains(bottom)) {
			p->drawLine(opt.rect.bottomLeft(), opt.rect.bottomRight());
		}
	} else {
		style->drawPrimitive(QStyle::PE_PanelItemViewItem, &o, p, o.widget);
	}
	p->restore();

	// Highlight the current playing item
	QFont font = settings->font(Settings::PLAYLIST);
	if (_playlist->mediaPlaylist()->currentIndex() == index.row() && _playlist->mediaPlayer().data()->state() != QMediaPlayer::StoppedState) {
		font.setBold(true);
		font.setItalic(true);
	}
	p->setFont(font);

	// Check if font color should be inverted
	QColor hiColor = settings->customColors(QPalette::Highlight);
	p->save();
	if (hiColor.value() < 128 && o.state.testFlag(QStyle::State_Selected)) {
		p->setPen(o.palette.highlightedText().color());
	}

	QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &o, o.widget);
	QString text;
	switch (index.column()) {
	case Playlist::TRACK_NUMBER:
	case Playlist::LENGTH:
	case Playlist::YEAR:
		text = QFontMetrics(font).elidedText(index.data().toString(), o.textElideMode, textRect.width());
		style->drawItemText(p, textRect, Qt::AlignCenter, o.palette, true, text);
		break;
	case Playlist::TITLE:
	case Playlist::ALBUM:
	case Playlist::ARTIST:
		text = QFontMetrics(font).elidedText(index.data().toString(), o.textElideMode, textRect.width());
		style->drawItemText(p, textRect, Qt::AlignLeft | Qt::AlignVCenter, o.palette, true, text);
		break;
	case Playlist::RATINGS:
		if (index.data().canConvert<StarRating>() || opt.state.testFlag(QStyle::State_Selected)) {
			StarRating r = index.data().value<StarRating>();
			r.paintStars(p, opt);
		}
		break;
	}
	p->restore();
}

