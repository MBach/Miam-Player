#include "tageditortablewidget.h"

#include <settings.h>
#include <filehelper.h>
#include <taglib/fileref.h>

#include "../styling/miamstyleditemdelegate.h"
#include "../treeview.h"

#include <QDir>
#include <QScrollBar>

#include <QtDebug>

TagEditorTableWidget::TagEditorTableWidget(QWidget *parent) :
	QTableWidget(parent)
{
	this->setItemDelegate(new MiamStyledItemDelegate(this, false));
	this->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

	/// XXX delegate should be improved because this piece of code has to be duplicated
	connect(this->selectionModel(), &QItemSelectionModel::selectionChanged, [=](const QItemSelection &, const QItemSelection &) {
		this->setDirtyRegion(QRegion(this->viewport()->rect()));
	});
	QList<QScrollBar*> scrollBars = QList<QScrollBar*>() << horizontalScrollBar() << verticalScrollBar();
	for (QScrollBar *scrollBar : scrollBars) {
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
	static const QStringList keys = (QStringList() << "FILENAME" << "ABSPATH" << "TITLE" << "ARTIST" << "ARTISTALBUM"
		//<< "ALBUM" << "TRACKNUMBER" << "DISC" << "YEAR" << "GENRE" << "COMMENT" << "COVER";
		<< "ALBUM" << "TRACKNUMBER" << "DISC" << "YEAR" << "GENRE" << "COMMENT");
	for (int column = 0; column < this->columnCount(); column++) {
		QTableWidgetItem *header = new QTableWidgetItem();
		header->setData(KEY, keys.at(column));
		this->setHorizontalHeaderItem(column, header);
	}
}

void TagEditorTableWidget::resetTable()
{
	this->setSortingEnabled(false);

	QMapIterator<int, QString> it(_indexes);
	while (it.hasNext()) {
		it.next();
		int row = it.key();
		QFileInfo fileInfo(it.value());

		FileHelper fh(fileInfo.absoluteFilePath());

		// Reload info
		int column = -1;
		QTableWidgetItem *filename = this->item(row, ++column);
		++column; // path cannot be changed
		QTableWidgetItem *title = this->item(row, ++column);
		QTableWidgetItem *artist = this->item(row, ++column);
		QTableWidgetItem *artistAlbum = this->item(row, ++column);
		QTableWidgetItem *album = this->item(row, ++column);
		QTableWidgetItem *trackNumber = this->item(row, ++column);
		QTableWidgetItem *disc = this->item(row, ++column);
		QTableWidgetItem *year = this->item(row, ++column);
		QTableWidgetItem *genre = this->item(row, ++column);
		QTableWidgetItem *comment = this->item(row, ++column);

		filename->setText(fileInfo.fileName());
		title->setText(fh.title());
		artist->setText(fh.artist());
		artistAlbum->setText(fh.artistAlbum());
		album->setText(fh.album());
		if (fh.trackNumber() != "00") {
			trackNumber->setText(fh.trackNumber());
		}
		int d = fh.discNumber(true);
		d > 0 ? disc->setText(QString::number(d)) :	disc->setText("");
		year->setText(fh.year());
		genre->setText(fh.genre());
		comment->setText(fh.comment());
	}
	this->setSortingEnabled(true);
	this->sortItems(0);
	this->sortItems(1);
}

void TagEditorTableWidget::updateCellData(int row, int column, const QString &text)
{
	QTableWidgetItem *i = this->item(row, column);
	i->setText(text);
	i->setData(MODIFIED, true);
}

void TagEditorTableWidget::updateColumnData(int column, const QString &text)
{
	for (QModelIndex index : selectionModel()->selectedRows(column)) {
		QTableWidgetItem *i = this->itemFromIndex(index);
		i->setText(text);
		i->setData(MODIFIED, true);
	}
}

/** Add items to the table in order to edit them. */
bool TagEditorTableWidget::addItemsToEditor(const QStringList &tracks, QMap<int, Cover*> &covers)
{
	QSet<QPair<QString, QString>> artistAlbumSet;
	for (QString track : tracks) {
		FileHelper fh(track);
		if (!fh.isValid()) {
			continue;
		}

		/// XXX: warning, this information is difficult to find even if public
		QTableWidgetItem *fileName = new QTableWidgetItem(fh.fileInfo().fileName());
		fileName->setData(Qt::UserRole, fh.fileInfo().absoluteFilePath());

		// The second column is not editable
		QTableWidgetItem *absPath = new QTableWidgetItem(QDir::toNativeSeparators(fh.fileInfo().path()));
		absPath->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

		QTableWidgetItem *title = new QTableWidgetItem(fh.title());
		QTableWidgetItem *artist = new QTableWidgetItem(fh.artist());
		QTableWidgetItem *artistAlbum = new QTableWidgetItem(fh.artistAlbum());
		QTableWidgetItem *album = new QTableWidgetItem(fh.album());
		QTableWidgetItem *trackNumber = nullptr;
		if (fh.trackNumber() == "00") {
			trackNumber = new QTableWidgetItem();
		} else {
			trackNumber = new QTableWidgetItem(fh.trackNumber());
		}

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
		_indexes.insert(row, fh.fileInfo().absoluteFilePath());

		/// XXX is it really necessary to extract cover in this class?
		/// It might be better to build a fileHelper outside, in the container (TagEditor), and iterate 2 times
		/// One in this class, one in TagEditor class ? But here is quite easy!
		/*Cover *cover = fh.extractCover();
		if (cover != nullptr && !cover->byteArray().isEmpty()) {
			covers.insert(row, cover);
		}*/
	}
	return (artistAlbumSet.size() == 1);
}

/** Redefined. */
void TagEditorTableWidget::clear()
{
	while (rowCount() > 0) {
		this->removeRow(0);
	}
	_indexes.clear();
	this->setSortingEnabled(false);
}
