#include "libraryitemdelegate.h"

#include "settings.h"
#include "stareditor.h"
#include "starrating.h"
#include "libraryitem.h"
#include "librarytreeview.h"

#include <QtDebug>

int LibraryItemDelegate::maxStars=5;


LibraryItemDelegate::LibraryItemDelegate(QObject *parent) :
	QStyledItemDelegate(parent)
{
	titleRect = new QRect();
	starsRect = new QRect();
	starEditor = new StarEditor();

	_stars = 3;

	favIcon.addFile(":/icons/favorite");
}

LibraryItemDelegate::~LibraryItemDelegate()
{
	delete titleRect;
	delete starsRect;
	delete starEditor;
}

#include <QStylePainter>

void LibraryItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	/// FIXME: when creating star editor, do a big code review
	/*if (qVariantCanConvert<StarRating>(index.data(LibraryItem::STAR_RATING))) {

		StarRating starRating = qVariantValue<StarRating>(index.data(LibraryItem::STAR_RATING));

		int titleRectWidth = option.rect.width();
		//int starsRectWidth = starRating.starCount() * 16;
		const QStandardItemModel *m = qobject_cast<const QStandardItemModel *>(index.model());
		QStandardItem *item = m->itemFromIndex(index);
		QFontMetrics fm(item->font());
		qDebug() << fm.height();
		titleRect->setRect(option.rect.x(), option.rect.y(), titleRectWidth, fm.height());
		//starsRect->setRect(option.rect.x()+option.rect.width()/2, option.rect.y(), starsRectWidth, option.rect.height());

		QStyleOptionViewItemV4 textViewItem(option);
		textViewItem.rect = *titleRect;
		QStyledItemDelegate::paint(painter, textViewItem, index);

		if (Settings::getInstance()->isStarDelegates()) {
			painter->save();
			//painter->translate(titleRect->width(), 0);
			//starRating.paint(painter, titleRect, option.palette, StarRating::ReadOnly);

			painter->setRenderHint(QPainter::Antialiasing, true);
			painter->setPen(Qt::NoPen);

			if (ReadOnly == Editable) {
				painter->setBrush(option.palette.highlight());
			} else {
				painter->setBrush(option.palette.foreground());
			}

			QRect favIconRect = *titleRect;
			favIconRect.setHeight(16);

			for (int i=maxStars; i>0; i--) {
				if (0 < i && i < maxStars) {
					painter->translate(-1.0 * (favIcon.actualSize(favIconRect.size()).width()+3.0), 0);
				}
				if (i > starRating.starCount()) {
					qDebug() << "painting";
					favIcon.paint(painter, favIconRect, Qt::AlignRight, QIcon::Disabled);
				} else {
					favIcon.paint(painter, favIconRect, Qt::AlignRight, QIcon::Normal);
				}
			}
			painter->restore();
		}
	} else*/
	if (index.data(LibraryItem::MEDIA_TYPE).toInt() == LibraryModel::LETTER) {
		QStyleOptionViewItemV4 o = option;
		o.state = QStyle::State_None;
		o.font.setBold(true);
		QPointF p1 = o.rect.bottomLeft(), p2 = o.rect.bottomRight();
		p1.setX(p1.x() + 2);
		p2.setX(p2.x() - 2);
		painter->setPen(Qt::gray);
		painter->drawLine(p1, p2);
		QStyledItemDelegate::paint(painter, o, index);
	} else {
		// Remove the dotted rectangle from the item that has focus
		QStyleOptionViewItemV4 o = option;
		o.state &= ~QStyle::State_HasFocus;
		QStyledItemDelegate::paint(painter, o, index);
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

/** Redefined to always display the same height for albums, even for those without one. */
QSize LibraryItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	Settings *settings = Settings::getInstance();
	if (settings->withCovers() && index.data(LibraryItem::MEDIA_TYPE).toInt() == LibraryModel::ALBUM) {
		return QSize(settings->coverSize(), settings->coverSize() + 2);
	} else {
		return QStyledItemDelegate::sizeHint(option, index);
	}
}

void LibraryItemDelegate::commitAndCloseEditor(QWidget */*e*/)
{
	qDebug() << "LibraryItemDelegate::commitAndCloseEditor(). StarEditor is NULL ?" << (starEditor == NULL);
	emit commitData(starEditor);
	emit closeEditor(starEditor);
}
