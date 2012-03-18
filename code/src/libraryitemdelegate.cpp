#include "libraryitemdelegate.h"

#include "stareditor.h"
#include "starrating.h"
#include "libraryitem.h"
#include "librarytreeview.h"

#include <QtDebug>

LibraryItemDelegate::LibraryItemDelegate(QObject *parent) :
	QStyledItemDelegate(parent)
{
	titleRect = new QRect();
	starsRect = new QRect();
	starEditor = new StarEditor();
}

LibraryItemDelegate::~LibraryItemDelegate()
{
	delete titleRect;
	delete starsRect;
}

void LibraryItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if (qVariantCanConvert<StarRating>(index.data(LibraryItem::STAR_RATING))) {
		StarRating starRating = qVariantValue<StarRating>(index.data(LibraryItem::STAR_RATING));

		int titleRectWidth = option.rect.width()/2;
		int starsRectWidth = option.rect.width()/2;
		titleRect->setRect(option.rect.x(), option.rect.y(), titleRectWidth, option.rect.height());
		starsRect->setRect(option.rect.x()+option.rect.width()/2, option.rect.y(), starsRectWidth, option.rect.height());

		painter->save();
		painter->translate(titleRect->width(), 0);
		starRating.paint(painter, titleRect, option.palette, StarRating::ReadOnly);
		painter->restore();

		QStyleOptionViewItemV4 textViewItem(option);
		textViewItem.rect = *titleRect;
		QStyledItemDelegate::paint(painter, textViewItem, index);
	} else {
		QStyledItemDelegate::paint(painter, option, index);
	}
}

QWidget* LibraryItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if (qVariantCanConvert<StarRating>(index.data(LibraryItem::STAR_RATING))) {
		starEditor->setMinimumHeight(20);
		starEditor->move(QPoint(100, 100));
		connect(starEditor, SIGNAL(editingFinished(QWidget *)), this, SLOT(commitAndCloseEditor(QWidget *)));
		setEditorData(starEditor, index);
		return starEditor;
	} else {
		return QStyledItemDelegate::createEditor(parent, option, index);
	}
}

void LibraryItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
	qDebug() << "setEditorData()";
	if (qVariantCanConvert<StarRating>(index.data(LibraryItem::STAR_RATING))) {
		StarRating starRating = qVariantValue<StarRating>(index.data(LibraryItem::STAR_RATING));
		//StarEditor *starEditor = qobject_cast<StarEditor *>(editor);
		starEditor->setStarRating(starRating);
	} else {
		QStyledItemDelegate::setEditorData(editor, index);
	}
}

void LibraryItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
	qDebug() << "setModelData()";
	if (qVariantCanConvert<StarRating>(index.data(LibraryItem::STAR_RATING))) {
		//StarEditor *starEditor = qobject_cast<StarEditor *>(editor);
		model->setData(index, qVariantFromValue(starEditor->starRating()), LibraryItem::STAR_RATING);
		qDebug() << "maj model ?" << model->data(index, LibraryItem::STAR_RATING).toInt();
	} else {
		QStyledItemDelegate::setModelData(editor, model, index);
	}
}



void LibraryItemDelegate::commitAndCloseEditor(QWidget */*e*/)
{
	qDebug() << "LibraryItemDelegate::commitAndCloseEditor(). StarEditor is NULL ?" << (starEditor == NULL);
	emit commitData(starEditor);
	emit closeEditor(starEditor);
}
