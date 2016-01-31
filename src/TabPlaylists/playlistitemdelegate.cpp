#include "playlistitemdelegate.h"

#include <model/sqldatabase.h>
#include <filehelper.h>
#include <settingsprivate.h>
#include "playlist.h"
#include "stareditor.h"

#include <QApplication>
#include <QDateTime>
#include <QHeaderView>
#include <QKeyEvent>
#include <QPainter>
#include <QStylePainter>

#include <QtDebug>

PlaylistItemDelegate::PlaylistItemDelegate(Playlist *playlist)
	: MiamStyledItemDelegate(playlist, true)
	, _playlist(playlist)
{}

/** Redefined. */
QWidget* PlaylistItemDelegate::createEditor(QWidget *p, const QStyleOptionViewItem &, const QModelIndex &index) const
{
	qDebug() << Q_FUNC_INFO;
	StarEditor *editor = new StarEditor(index, p);
	connect(editor, &StarEditor::editFinished, this, &PlaylistItemDelegate::commitAndClose);
	return editor;
}

bool PlaylistItemDelegate::eventFilter(QObject *object, QEvent *event)
{
	// Cancel input and close editor
	if (event->type() == QEvent::KeyPress) {
		QKeyEvent *ke = static_cast<QKeyEvent*>(event);
		if (ke->key() == Qt::Key_Escape) {
			auto starEditor = qobject_cast<StarEditor*>(object);
			StarRating sr = _playlist->model()->data(starEditor->index()).value<StarRating>();
			starEditor->starRating.setStarCount(sr.starCount());
			closeEditor(starEditor);
			return true;
		}
	} else if (event->type() == QEvent::FocusOut) {
		// Save value only if different from the moment where the editor was opened
		auto starEditor = qobject_cast<StarEditor*>(object);
		StarRating sr = _playlist->model()->data(starEditor->index()).value<StarRating>();
		if (starEditor->starRating.starCount() != sr.starCount()) {
			commitAndClose();
			return true;
		}
	}
	return MiamStyledItemDelegate::eventFilter(object, event);
}

void PlaylistItemDelegate::commitAndClose()
{
	SqlDatabase db;
	QStringList tracksToUpdate;
	QStringList tracksToUpdate2;
	// Multiple editors might have been opened by one, therefore it's required to commit and close all of them
	for (StarEditor *se : parent()->findChildren<StarEditor*>()) {
		QMediaContent mediaContent = _playlist->mediaPlaylist()->media(se->index().row());
		QString fileName = QString(QFile::encodeName(mediaContent.canonicalUrl().toLocalFile()));
		FileHelper fh(fileName);
		fh.setRating(se->starRating.starCount());
		commitData(se);
		closeEditor(se);
		tracksToUpdate << fileName;
		tracksToUpdate2 << QString();
	}
	db.updateTracks(tracksToUpdate, tracksToUpdate2);
}

/** Redefined. */
void PlaylistItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
	StarEditor *starEditor = qobject_cast<StarEditor *>(editor);
	model->setData(index, QVariant::fromValue(starEditor->starRating));
	starEditor->deleteLater();
}

/** Redefined. */
void PlaylistItemDelegate::paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
	QStyleOptionViewItem o(opt);
	initStyleOption(&o, index);

	QStyle *style = QApplication::style();
	QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &o, o.widget);

	if (_playlist->horizontalHeader()->visualIndex(index.column()) == 0) {
		textRect = textRect.adjusted(1, 0, 0, 0);
		o.rect = textRect;
	}

	MiamStyledItemDelegate::paint(p, o, index);

	// Highlight the current playing item
	QFont font = SettingsPrivate::instance()->font(SettingsPrivate::FF_Playlist);
	if (_playlist->mediaPlaylist()->currentIndex() == index.row() && _playlist->mediaPlayer()->state() != QMediaPlayer::StoppedState) {
		font.setBold(true);
		font.setItalic(true);
	}
	p->setFont(font);

	p->save();
	if (o.state.testFlag(QStyle::State_Selected)) {
		if ((opt.palette.highlight().color().lighter(160).saturation() - opt.palette.highlightedText().color().saturation()) < 128) {
			p->setPen(opt.palette.text().color());
		} else {
			p->setPen(opt.palette.highlightedText().color());
		}
	}

	QString text;
	switch (index.column()) {

	case Playlist::COL_LENGTH:
		if (index.data().toInt() >= 0) {
			text = QDateTime::fromTime_t(index.data().toInt()).toString("m:ss");
			text = p->fontMetrics().elidedText(text, o.textElideMode, textRect.width());
			style->drawItemText(p, textRect, Qt::AlignCenter, o.palette, true, text);
		}
		break;
	case Playlist::COL_TRACK_NUMBER:
	case Playlist::COL_YEAR:
		text = p->fontMetrics().elidedText(index.data().toString(), o.textElideMode, textRect.width());
		style->drawItemText(p, textRect, Qt::AlignCenter, o.palette, true, text);
		break;
	case Playlist::COL_RATINGS:
		if (index.data().canConvert<StarRating>() || opt.state.testFlag(QStyle::State_Selected)) {
			StarRating r = index.data().value<StarRating>();
			r.paintStars(p, opt);
		}
		break;
	case Playlist::COL_ICON:{
		//QRect iconRect = style->subElementRect(QStyle::SE_ItemViewItemDecoration, &o, o.widget);
		text = p->fontMetrics().elidedText(index.data().toString(), o.textElideMode, textRect.width());
		QSize iconSize(textRect.height() * 0.8, textRect.height() * 0.8);
		/// FIXME
		//style->drawItemText(p, textRect, Qt::AlignCenter, o.palette, true, text);
		style->drawItemPixmap(p, o.rect, Qt::AlignCenter, o.icon.pixmap(iconSize));
		break;
	}
	case Playlist::COL_TITLE:
	case Playlist::COL_ALBUM:
	case Playlist::COL_ARTIST:
	default:
		text = p->fontMetrics().elidedText(index.data().toString(), o.textElideMode, textRect.width());
		if (QApplication::isLeftToRight()) {
			textRect.adjust(2, 0, 0, 0);
		} else {
			textRect.adjust(0, 0, -2, 0);
		}
		style->drawItemText(p, textRect, Qt::AlignLeft | Qt::AlignVCenter, o.palette, true, text);
		break;
	}
	p->restore();
}

