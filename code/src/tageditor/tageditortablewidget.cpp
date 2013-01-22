#include "tageditortablewidget.h"

#include "library/libraryitem.h"
#include "nofocusitemdelegate.h"
#include "settings.h"
#include "treeview.h"

#include <QScrollBar>

#include <QtDebug>

using namespace Phonon;

TagEditorTableWidget::TagEditorTableWidget(QWidget *parent) :
	QTableWidget(parent)
{
	Settings *settings = Settings::getInstance();
	this->setStyleSheet(settings->styleSheet(this));
	/// FIXME: not displayed using styleSheet...
    //this->horizontalScrollBar()->setStyleSheet(settings->styleSheet(horizontalScrollBar()));
	this->verticalScrollBar()->setStyleSheet(settings->styleSheet(verticalScrollBar()));
	this->setItemDelegate(new NoFocusItemDelegate(this));
}

/** It's not possible to initialize header in the constructor. The object has to be instantiated completely first. */
void TagEditorTableWidget::init()
{
	// Always keep the same number of columns with this taglist
	QStringList keys = (QStringList() << "FILENAME" << "ABSPATH" << "TITLE" << "ARTIST" << "ARTISTALBUM");
	keys << "ALBUM" << "TRACKNUMBER" << "DISC" << "DATE" << "GENRE" << "COMMENT" << "COVER";
	for (int column = 0; column < this->columnCount(); column++) {
		QTableWidgetItem *header = this->horizontalHeaderItem(column);
		if (!header) {
			header = new QTableWidgetItem(0);
			this->setHorizontalHeaderItem(column, header);
		}
		header->setData(KEY, keys.at(column));
	}

	// Hide the cover column.
	// It is not displayed in the table, only when one clicks on an item, before the comboboxes.
	this->setColumnHidden(this->columnCount() - 1, true);
}

void TagEditorTableWidget::updateColumnData(int column, QString text)
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
		MediaSource source(fileInfo.absoluteFilePath());
		if (source.type() != MediaSource::Invalid) {
			TagLib::FileRef f(source.fileName().toLocal8Bit().data());

			FileHelper fh(f, it.value().data(LibraryItem::SUFFIX).toInt());

			// Reload info
			int column = 1;
			QTableWidgetItem *title = this->item(row, ++column);
			QTableWidgetItem *artist = this->item(row, ++column);
			QTableWidgetItem *artistAlbum = this->item(row, ++column);
			QTableWidgetItem *album = this->item(row, ++column);
			QTableWidgetItem *trackNumber = this->item(row, ++column);
			//QTableWidgetItem *disc = this->item(row, ++column);
			QTableWidgetItem *year = this->item(row, ++column);
			QTableWidgetItem *genre = this->item(row, ++column);
			QTableWidgetItem *comment = this->item(row, ++column);
			QTableWidgetItem *cover = this->item(row, ++column);

			title->setText(f.tag()->title().toCString());
			artist->setText(f.tag()->artist().toCString());
			artistAlbum->setText(fh.artistAlbum().toCString());
			album->setText(f.tag()->album().toCString());
			trackNumber->setText(QString::number(f.tag()->track()));
			//disc->setText(discNumber.toCString());
			year->setText(QString::number(f.tag()->year()));
			genre->setText(f.tag()->genre().toCString());
			comment->setText(f.tag()->comment().toCString());
			cover->setData(Qt::EditRole, fh.extractCover());
		}
		row++;
	}
	this->setSortingEnabled(true);
	this->sortItems(0);
	this->sortItems(1);
}

/** Add items to the table in order to edit them. */
bool TagEditorTableWidget::addItemsToEditor(const QList<QPersistentModelIndex> &indexList)
{
	QSet<QPair<QString, QString> > artistAlbumSet;
	foreach (QPersistentModelIndex index, indexList) {
		QString absFilePath = TreeView::absFilePath(index);
		MediaSource source(absFilePath);
		if (source.type() != MediaSource::Invalid) {
			TagLib::FileRef f(source.fileName().toLocal8Bit().data());
			indexes.insert(absFilePath, index);

			FileHelper fh(f, index.data(LibraryItem::SUFFIX).toInt());

			// The first two columns are not editable
			// It may changes in the future for the first one (the filename)
			QFileInfo qFileInfo(absFilePath);
			QTableWidgetItem *fileName = new QTableWidgetItem(qFileInfo.fileName());
			fileName->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
			fileName->setData(LibraryItem::SUFFIX, fh.type());
			QTableWidgetItem *absPath = new QTableWidgetItem(qFileInfo.absolutePath());
			absPath->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);

			QTableWidgetItem *title = new QTableWidgetItem(f.tag()->title().toCString());
			QTableWidgetItem *artist = new QTableWidgetItem(f.tag()->artist().toCString());
			QTableWidgetItem *artistAlbum = new QTableWidgetItem(fh.artistAlbum().toCString());
			QTableWidgetItem *album = new QTableWidgetItem(f.tag()->album().toCString());
			/// FIXME: is there a way to extract String = "01" instead of int = 1 ?
			QString track = QString("%1").arg(f.tag()->track(), 2, 10, QChar('0')).toUpper();
			QTableWidgetItem *trackNumber = new QTableWidgetItem(track);
			//QTableWidgetItem *disc = new QTableWidgetItem(discNumber.toCString());
			QTableWidgetItem *disc = new QTableWidgetItem("");
			QTableWidgetItem *year = new QTableWidgetItem(QString::number(f.tag()->year()));
			QTableWidgetItem *genre = new QTableWidgetItem(f.tag()->genre().toCString());
			QTableWidgetItem *comment = new QTableWidgetItem(f.tag()->comment().toCString());
			QTableWidgetItem *cover = new QTableWidgetItem(QTableWidgetItem::Type);
			cover->setData(Qt::EditRole, fh.extractCover());

			QList<QTableWidgetItem*> items;
			items << fileName << absPath << title << artist << artistAlbum << album << trackNumber << disc << year << genre << comment << cover;

			// Check if there's only one album in the list, used for the context menu of the cover
			artistAlbumSet.insert(qMakePair(artist->text(), album->text()));

			// Create a new row with right data
			int row = rowCount();
			this->insertRow(row);
			for (int column = 0; column < items.size(); column++) {
				this->setItem(row, column, items.at(column));
			}
		}
	}
	return (artistAlbumSet.size() == 1);
}

/** Redefined. */
void TagEditorTableWidget::clear()
{
	while (rowCount() > 0) {
		this->removeRow(0);
	}
	indexes.clear();
	this->setSortingEnabled(false);
}
