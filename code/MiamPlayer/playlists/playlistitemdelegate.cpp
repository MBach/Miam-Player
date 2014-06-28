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
	MiamStyledItemDelegate(playlist, true), _playlist(playlist)
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
	MiamStyledItemDelegate::paint(p, opt, index);

	QStyleOptionViewItem o(opt);
	initStyleOption(&o, index);

	// Highlight the current playing item
	QFont font = Settings::getInstance()->font(Settings::FF_Playlist);
	if (_playlist->mediaPlaylist()->currentIndex() == index.row() && _playlist->mediaPlayer().data()->state() != QMediaPlayer::StoppedState) {
		font.setBold(true);
		font.setItalic(true);
	}
	p->setFont(font);

	p->save();
	if (o.state.testFlag(QStyle::State_Selected)) {
		p->setPen(o.palette.highlightedText().color());
	}

	QStyle *style = QApplication::style();
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

