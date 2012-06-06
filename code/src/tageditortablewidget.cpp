#include "tageditortablewidget.h"
#include "settings.h"
#include "libraryitem.h"

#include <fileref.h>
#include <tag.h>
#include <phonon>

#include <QtDebug>

using namespace Phonon;

TagEditorTableWidget::TagEditorTableWidget(QWidget *parent) :
	QTableWidget(parent)
{

}

void TagEditorTableWidget::addItemFromLibrary(const QPersistentModelIndex &index)
{
	QString path = Settings::getInstance()->musicLocations().at(index.data(LibraryItem::IDX_TO_ABS_PATH).toInt()).toString();
	QString name = index.data(LibraryItem::REL_PATH_TO_MEDIA).toString();
	QFileInfo fileInfo(path + name);
	MediaSource source(fileInfo.absoluteFilePath());

	// Extract relevant fields
	if (source.type() != MediaSource::Invalid) {
		TagLib::FileRef f(source.fileName().toLocal8Bit().data());

		QTableWidgetItem *baseName = new QTableWidgetItem(fileInfo.completeBaseName());
		QTableWidgetItem *absPath = new QTableWidgetItem(fileInfo.absolutePath());
		QTableWidgetItem *title = new QTableWidgetItem(f.tag()->title().toCString());
		QTableWidgetItem *artist = new QTableWidgetItem(f.tag()->artist().toCString());
		QTableWidgetItem *artistAlbum = new QTableWidgetItem("Artist Album");
		QTableWidgetItem *album = new QTableWidgetItem(f.tag()->album().toCString());
		QTableWidgetItem *disc = new QTableWidgetItem("Disc");
		QTableWidgetItem *year = new QTableWidgetItem(QString::number(f.tag()->year()));
		QTableWidgetItem *genre = new QTableWidgetItem(f.tag()->genre().toCString());
		QTableWidgetItem *comment = new QTableWidgetItem(f.tag()->comment().toCString());

		QList<QTableWidgetItem*> items;
		items << baseName << absPath << title << artist << artistAlbum << album << disc << year << genre << comment;

		// Create a new row with right data
		int r = rowCount();
		this->insertRow(r);
		for (int column = 0; column < items.size(); column++) {
			QTableWidgetItem *item = items.at(column);
			this->setItem(r, column, item);
		}
	}
}
