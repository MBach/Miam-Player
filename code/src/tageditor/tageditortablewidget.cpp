#include "tageditortablewidget.h"
#include "settings.h"
#include "library/libraryitem.h"

#include <fileref.h>
#include <id3v2tag.h>
#include <mpegfile.h>
#include <tag.h>
#include <tlist.h>
#include <textidentificationframe.h>
#include <tstring.h>
#include <phonon>

#include <QtDebug>

using namespace Phonon;

using namespace TagLib;

TagEditorTableWidget::TagEditorTableWidget(QWidget *parent) :
	QTableWidget(parent)
{}

void TagEditorTableWidget::addItemFromLibrary(const QPersistentModelIndex &index)
{
	QString path = Settings::getInstance()->musicLocations().at(index.data(LibraryItem::IDX_TO_ABS_PATH).toInt()).toString();
	QString name = index.data(LibraryItem::REL_PATH_TO_MEDIA).toString();
	QFileInfo fileInfo(path + name);
	MediaSource source(fileInfo.absoluteFilePath());

	// Extract relevant fields
	if (source.type() != MediaSource::Invalid) {
		TagLib::FileRef f(source.fileName().toLocal8Bit().data());

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
		int r = rowCount();
		this->insertRow(r);
		for (int column = 0; column < items.size(); column++) {
			QTableWidgetItem *item = items.at(column);
			this->setItem(r, column, item);
		}
	}
}
