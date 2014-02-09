#include <QtWidgets>

#include "stardelegate.h"
#include "stareditor.h"
#include "starrating.h"

#include <settings.h>
#include <filehelper.h>

StarDelegate::StarDelegate(QMediaPlaylist *parent)
	: QStyledItemDelegate(parent), _mediaPlaylist(parent)
{

}

/** Redefined. */
QWidget* StarDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const
{
	StarEditor *editor = new StarEditor(parent);
	connect(editor, &StarEditor::editingFinished, [=]() {
		//emit commitData(editor);
		//qDebug() << "saving file" << index;
		//FileHelper fh;
		//fh.file()->save();
		//emit aboutToUpdateRatings(index);
		qDebug() << "ratings to update" << _mediaPlaylist->media(index.row()).canonicalUrl();
		QMediaContent mediaContent = _mediaPlaylist->media(index.row());
		FileHelper fh(QString(QFile::encodeName(mediaContent.canonicalUrl().toLocalFile())));
		fh.setRating(editor->starRating().starCount());
		delete editor;
	});
	return editor;
}

/** Redefined. */
void StarDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	// Removes the dotted rectangle
	QStyleOptionViewItem opt = option;
	opt.state &= ~QStyle::State_HasFocus;

	if (opt.state.testFlag(QStyle::State_Selected) && opt.state.testFlag(QStyle::State_Active)) {
		painter->fillRect(opt.rect, option.palette.highlight());
	} else {
		if (!Settings::getInstance()->colorsAlternateBG() || index.row() % 2 == 0) {
			painter->fillRect(opt.rect, option.palette.base());
		} else {
			painter->fillRect(opt.rect, option.palette.alternateBase());
		}
	}
	if (index.data().canConvert<StarRating>()) {
		StarRating starRating = qvariant_cast<StarRating>(index.data());
		starRating.paint(painter, opt, StarRating::ReadOnly);
	} else if (opt.state & QStyle::State_Selected) {
		StarRating starRating(StarRating::maxStarCount);
		starRating.paint(painter, opt, StarRating::NoStarsYet);
	}
}

/** Redefined. */
void StarDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
	qDebug() << Q_FUNC_INFO << editor;
	StarRating starRating = qvariant_cast<StarRating>(index.data());
	StarEditor *starEditor = qobject_cast<StarEditor *>(editor);
	starEditor->setStarRating(starRating);
}

/** Redefined. */
void StarDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
	qDebug() << Q_FUNC_INFO;
	StarEditor *starEditor = qobject_cast<StarEditor *>(editor);
	model->setData(index, QVariant::fromValue(starEditor->starRating()));
}

/** Redefined. */
QSize StarDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if (index.data().canConvert<StarRating>()) {
		return option.rect.size();
	} else {
		return QStyledItemDelegate::sizeHint(option, index);
	}
}
