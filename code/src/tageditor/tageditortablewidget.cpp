#include "tageditortablewidget.h"
#include "settings.h"
#include "library/libraryitem.h"

#include <QScrollBar>

#include <id3v2tag.h>
#include <mpegfile.h>
#include <tag.h>
#include <tlist.h>
#include <textidentificationframe.h>
#include <tstring.h>
#include <phonon>

#include <QtDebug>

#include <QApplication>
#include <QHeaderView>

using namespace Phonon;

using namespace TagLib;

TagEditorTableWidget::TagEditorTableWidget(QWidget *parent) :
	QTableWidget(parent)
{
	Settings *settings = Settings::getInstance();
	this->setStyleSheet(settings->styleSheet(this));
	this->horizontalScrollBar()->setStyleSheet(settings->styleSheet(horizontalScrollBar()));
	this->verticalScrollBar()->setStyleSheet(settings->styleSheet(verticalScrollBar()));
}

void TagEditorTableWidget::init()
{
	// Always keep the same number of columns with this taglist
	QStringList keys = (QStringList() << "FILENAME" << "ABSPATH" << "TITLE" << "ARTIST" << "ARTISTALBUM");
	keys << "ALBUM" << "TRACKNUMBER" << "DISC" << "DATE" << "GENRE" << "COMMENT";
	for (int column = 0; column < this->columnCount(); column++) {
		QTableWidgetItem *header = this->horizontalHeaderItem(column);
		if (!header) {
			header = new QTableWidgetItem(0);
			this->setHorizontalHeaderItem(column, header);
		}
		header->setData(KEY, keys.at(column));
	}
}

void TagEditorTableWidget::updateColumnData(int column, QString text)
{
	foreach (QModelIndex index, selectionModel()->selectedRows(column)) {
		QTableWidgetItem *item = itemFromIndex(index);
		item->setText(text);
		item->setData(MODIFIED, true);
	}
}

void TagEditorTableWidget::fillTable(const QFileInfo fileInfo, const TagLib::FileRef f)
{
	// Extract relevant fields
	String artAlb, discNumber;
	// If the file is a MP3
	if (TagLib::MPEG::File* file = dynamic_cast<TagLib::MPEG::File*>(f.file())) {
		if (file->ID3v2Tag()) {
			if (!file->ID3v2Tag()->frameListMap()["TPE2"].isEmpty()) {
				artAlb = file->ID3v2Tag()->frameListMap()["TPE2"].front()->toString();
			}
			if (!file->ID3v2Tag()->frameListMap()["TPOS"].isEmpty()) {
				discNumber = file->ID3v2Tag()->frameListMap()["TPOS"].front()->toString();
			}
		}
	}

	QTableWidgetItem *fileName = new QTableWidgetItem(fileInfo.fileName());
	QTableWidgetItem *absPath = new QTableWidgetItem(fileInfo.absolutePath());
	absPath->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
	QTableWidgetItem *title = new QTableWidgetItem(f.tag()->title().toCString());
	QTableWidgetItem *artist = new QTableWidgetItem(f.tag()->artist().toCString());
	QTableWidgetItem *artistAlbum = new QTableWidgetItem(artAlb.toCString());
	QTableWidgetItem *album = new QTableWidgetItem(f.tag()->album().toCString());
	/// FIXME: is there a way to extract String = "01" instead of int = 1 ?
	QTableWidgetItem *trackNumber = new QTableWidgetItem(QString::number(f.tag()->track()));
	QTableWidgetItem *disc = new QTableWidgetItem(discNumber.toCString());
	QTableWidgetItem *year = new QTableWidgetItem(QString::number(f.tag()->year()));
	QTableWidgetItem *genre = new QTableWidgetItem(f.tag()->genre().toCString());
	QTableWidgetItem *comment = new QTableWidgetItem(f.tag()->comment().toCString());

	QList<QTableWidgetItem*> items;
	items << fileName << absPath << title << artist << artistAlbum << album << trackNumber << disc << year << genre << comment;

	// Create a new row with right data
	int row = rowCount();
	this->insertRow(row);
	for (int column = 0; column < items.size(); column++) {
		this->setItem(row, column, items.at(column));
	}
}

void TagEditorTableWidget::resetTable()
{
	// Delete items
	/// XXX: is it really necessary to delete-rebuild every time?
	/// It might be better to use a clear-populate mechanism, therefore we can keep the selection model
	for (int i = 0; i < tracks.size(); i++) {
		this->removeRow(0);
	}

	// Reload info from the list
	for (int i = 0; i < tracks.size(); i++) {
		this->fillTable(files.at(i), tracks.at(i));
	}
}

void TagEditorTableWidget::addItemFromLibrary(const QPersistentModelIndex &index)
{
	QString path = Settings::getInstance()->musicLocations().at(index.data(LibraryItem::IDX_TO_ABS_PATH).toInt()).toString();
	QString name = index.data(LibraryItem::REL_PATH_TO_MEDIA).toString();
	QFileInfo fileInfo(path + name);
	MediaSource source(fileInfo.absoluteFilePath());

	if (source.type() != MediaSource::Invalid) {
		TagLib::FileRef f(source.fileName().toLocal8Bit().data());
		// Wow, some code cleanup should really be done here...
		files.append(fileInfo);
		tracks.append(f);
		indexes.append(index);
		this->fillTable(fileInfo, f);
	}

}
