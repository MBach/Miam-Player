#include "tageditortablewidget.h"

#include <settings.h>
#include <filehelper.h>
#include <3rdparty/taglib/fileref.h>

#include "../styling/miamstyleditemdelegate.h"
#include "../treeview.h"

#include <QScrollBar>

#include <QtDebug>

TagEditorTableWidget::TagEditorTableWidget(QWidget *parent) :
	QTableWidget(parent)
{
	this->setItemDelegate(new MiamStyledItemDelegate(this, false));

	/// XXX delegate should be improved because this piece of code has to be duplicated
    connect(this->selectionModel(), &QItemSelectionModel::selectionChanged, [=](const QItemSelection &, const QItemSelection &) {
		this->setDirtyRegion(QRegion(this->viewport()->rect()));
	});
	QList<QScrollBar*> scrollBars = QList<QScrollBar*>() << horizontalScrollBar() << verticalScrollBar();
	foreach (QScrollBar *scrollBar, scrollBars) {
		connect(scrollBar, &QScrollBar::sliderPressed, [=]() { viewport()->update(); });
		connect(scrollBar, &QScrollBar::sliderMoved, [=]() { viewport()->update(); });
		connect(scrollBar, &QScrollBar::sliderReleased, [=]() { viewport()->update(); });
	}
	///
}

/** It's not possible to initialize header in the constructor. The object has to be instantiated completely first. */
void TagEditorTableWidget::init()
{
	// Always keep the same number of columns with this taglist
	QStringList keys = (QStringList() << "FILENAME" << "ABSPATH" << "TITLE" << "ARTIST" << "ARTISTALBUM");
	//keys << "ALBUM" << "TRACKNUMBER" << "DISC" << "DATE" << "GENRE" << "COMMENT" << "COVER";
	keys << "ALBUM" << "TRACKNUMBER" << "DISC" << "DATE" << "GENRE" << "COMMENT";
	for (int column = 0; column < this->columnCount(); column++) {
		QTableWidgetItem *header = new QTableWidgetItem();
		header->setData(KEY, keys.at(column));
		this->setHorizontalHeaderItem(column, header);
	}
}

void TagEditorTableWidget::updateColumnData(int column, const QString &text)
{
	foreach (QModelIndex index, selectionModel()->selectedRows(column)) {
		QTableWidgetItem *item = itemFromIndex(index);
		item->setText(text);
		item->setData(MODIFIED, true);
	}
}

void TagEditorTableWidget::resetTable()
{
	this->setSortingEnabled(false);

	QMapIterator<QString, QPersistentModelIndex> it(indexes);
	int row = 0;
	while (it.hasNext()) {
		it.next();
		QFileInfo fileInfo(it.key());

		FileHelper fh(fileInfo.absoluteFilePath());

		// Reload info
		int column = 1;
		QTableWidgetItem *title = this->item(row, ++column);
		QTableWidgetItem *artist = this->item(row, ++column);
		QTableWidgetItem *artistAlbum = this->item(row, ++column);
		QTableWidgetItem *album = this->item(row, ++column);
		QTableWidgetItem *trackNumber = this->item(row, ++column);
		QTableWidgetItem *disc = this->item(row, ++column);
		QTableWidgetItem *year = this->item(row, ++column);
		QTableWidgetItem *genre = this->item(row, ++column);
		QTableWidgetItem *comment = this->item(row, ++column);

		title->setText(fh.title());
		artist->setText(fh.artist());
		artistAlbum->setText(fh.artistAlbum());
		album->setText(fh.album());
		trackNumber->setText(fh.trackNumber());
		disc->setText(QString::number(fh.discNumber()));
		year->setText(fh.year());
		genre->setText(fh.genre());
		comment->setText(fh.comment());
		//}
		row++;
	}
	this->setSortingEnabled(true);
	this->sortItems(0);
	this->sortItems(1);
}

/** Add items to the table in order to edit them. */
bool TagEditorTableWidget::addItemsToEditor(const QStringList &tracks, QMap<int, Cover*> &covers)
{
	QSet<QPair<QString, QString> > artistAlbumSet;
	foreach (QString track, tracks) {
		FileHelper fh(track);

		// The first two columns are not editable
		// It may changes in the future for the first one (the filename)
		QTableWidgetItem *fileName = new QTableWidgetItem(fh.fileInfo().fileName());
		QTableWidgetItem *absPath = new QTableWidgetItem(fh.fileInfo().path());
		fileName->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		absPath->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

		QTableWidgetItem *title = new QTableWidgetItem(fh.title());
		QTableWidgetItem *artist = new QTableWidgetItem(fh.artist());
		QTableWidgetItem *artistAlbum = new QTableWidgetItem(fh.artistAlbum());
		QTableWidgetItem *album = new QTableWidgetItem(fh.album());
		QTableWidgetItem *trackNumber = new QTableWidgetItem(fh.trackNumber());
		QTableWidgetItem *disc;
		if (fh.discNumber() == 0) {
			disc = new QTableWidgetItem;
		} else {
			disc = new QTableWidgetItem(QString::number(fh.discNumber()));
		}
		QTableWidgetItem *year = new QTableWidgetItem(fh.year());
		QTableWidgetItem *genre = new QTableWidgetItem(fh.genre());
		QTableWidgetItem *comment = new QTableWidgetItem(fh.comment());

		QList<QTableWidgetItem*> items;
		items << fileName << absPath << title << artist << artistAlbum << album << trackNumber << disc << year << genre << comment;

		// Check if there's only one album in the list, used for the context menu of the cover
		artistAlbumSet.insert(qMakePair(artist->text(), album->text()));

		// Create a new row with right data
		int row = rowCount();
		this->insertRow(row);
		for (int column = 0; column < items.size(); column++) {
			this->setItem(row, column, items.at(column));
		}

		/// XXX is it really necessary to extract cover in this class?
		/// It might be better to build a fileHelper outside, in the container (TagEditor), and iterate 2 times
		/// One in this class, one in TagEditor class ? But here is quite easy!
		Cover *cover = fh.extractCover();
		if (cover != NULL && !cover->byteArray().isEmpty()) {
			covers.insert(row, cover);
		}
	}
	return (artistAlbumSet.size() == 1);
}

/** Redefined. */
void TagEditorTableWidget::clear()
{
	qDebug() << "TagEditorTableWidget::clear() 2";
	while (rowCount() > 0) {
		this->removeRow(0);
	}
	qDebug() << "TagEditorTableWidget::clear() 3";
	indexes.clear();
	qDebug() << "TagEditorTableWidget::clear() 4";
	this->setSortingEnabled(false);
}
