#include "tageditor.h"
#include "library/libraryitem.h"
#include "settings.h"

#include <QtDebug>

#include <QFile>
#include <QGraphicsPixmapItem>

#include <fileref.h>
#include <tag.h>

using namespace TagLib;

TagEditor::TagEditor(QWidget *parent) :
	QWidget(parent), atLeastOneItemChanged(false)
{
	setupUi(this);

	tagConverter = new TagConverter(this);

	int i = 1;
	combos.insert(++i, titleComboBox);
	combos.insert(++i, artistComboBox);
	combos.insert(++i, artistAlbumComboBox);
	combos.insert(++i, albumComboBox);
	combos.insert(++i, trackComboBox);
	combos.insert(++i, discComboBox);
	combos.insert(++i, yearComboBox);
	combos.insert(++i, genreComboBox);
	combos.insert(++i, commentComboBox);

	foreach (QComboBox *combo, combos.values()) {
		combo->addItem(tr("(Keep)"));
		combo->addItem(tr("(Delete)"));
		combo->setCurrentIndex(-1);
		//connect(combo, SIGNAL(editTextChanged(QString)), this, SLOT(activateSaveButton(QString)));
		connect(combo, SIGNAL(editTextChanged(QString)), this, SLOT(updateTable(QString)));
	}

	// Quit this widget when a request was send from this button
	connect(closeTagEditorButton, SIGNAL(clicked()), this, SLOT(close()));

	connect(saveChangesButton, SIGNAL(clicked()), this, SLOT(commitChanges()));
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(rollbackChanges()));

	// General case: when one is selecting multiple items
	connect(tagEditorWidget, SIGNAL(itemSelectionChanged()), this, SLOT(displayTags()));

	// Open the TagConverter to help tagging from Tag to File, or vice-versa
	connect(convertPushButton, SIGNAL(toggled(bool)), this, SLOT(toggleTagConverter(bool)));
}

/** Delete all rows. */
void TagEditor::clear()
{
	// QTableWidget::clear is not deleting rows? Need to be done manually
	while (tagEditorWidget->rowCount() > 0) {
		tagEditorWidget->removeRow(0);
	}
}

void TagEditor::beforeAddingItems()
{
	saveChangesButton->setEnabled(false);
	cancelButton->setEnabled(false);
	disconnect(tagEditorWidget, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(recordChanges(QTableWidgetItem*)));
}

void TagEditor::addItemFromLibrary(const QPersistentModelIndex &index)
{
	tagEditorWidget->addItemFromLibrary(index);
	// Test
	tracks.insert(tagEditorWidget->rowCount() - 1, index);
	filenames.insert(tagEditorWidget->rowCount() - 1, false);
	tags.insert(tagEditorWidget->rowCount() - 1, false);
}

void TagEditor::afterAddingItems()
{
	// It's possible to edit single items by double-clicking in the table
	connect(tagEditorWidget, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(recordChanges(QTableWidgetItem*)));
	tagEditorWidget->resizeColumnsToContents();
}

void TagEditor::activateSaveButton(QString)
{
	// Compare the tag temporarily stored in memory with the new input

	saveChangesButton->setEnabled(true);
}

/** Close this Widget and tells its parent to switch views. */
void TagEditor::close()
{
	emit closeTagEditor(false);
	saveChangesButton->setEnabled(false);
	cancelButton->setEnabled(false);
	atLeastOneItemChanged = false;
	tracks.clear();
	filenames.clear();
	tags.clear();
	QWidget::close();
}

void TagEditor::commitChanges()
{
	QMapIterator<int, bool> t(tags), f(filenames);
	// Apply tags first
	while (t.hasNext()) {
		t.next();
		if (t.value()) {
			qDebug() << "TODO write tags onto the hdd";
			int row = t.key();
			QPersistentModelIndex index = tracks.value(row);
			QString filePath = Settings::getInstance()->musicLocations().at(index.data(LibraryItem::IDX_TO_ABS_PATH).toInt()).toString();
			QString fileName = index.data(LibraryItem::REL_PATH_TO_MEDIA).toString();
			if (QFile::exists(filePath + fileName)) {

			}
		}
	}

	// Apply new filenames
	while (f.hasNext()) {
		f.next();
		if (f.value()) {
			int row = f.key();
			/// XXX: Settings::getInstance()->absoluteFilePath(QModelIndex);
			QPersistentModelIndex index = tracks.value(row);
			QString filePath = Settings::getInstance()->musicLocations().at(index.data(LibraryItem::IDX_TO_ABS_PATH).toInt()).toString();
			QString fileName = index.data(LibraryItem::REL_PATH_TO_MEDIA).toString();
			if (QFile::exists(filePath + fileName)) {
				//QTableWidgetItem *newFilename = tagEditorWidget->item(row, 0);
				//QTableWidgetItem *path = tagEditorWidget->item(row, 1);
				QFile file(filePath + fileName);
				//file.rename(path->data(Qt::DisplayRole).toString() + '/' + newFilename->data(Qt::DisplayRole).toString());
				file.close();
				//qDebug() << (filePath + fileName) << "was successfully renamed to" << newAbsFilepath;
			}
		}
	}
	if (!filenames.isEmpty()) {
		emit tracksRenamed();
	}
}

void TagEditor::rollbackChanges()
{
	qDebug() << "rollbackChanges necessary?";
}

void TagEditor::updateTable(QString text)
{
	qDebug() << sender()->objectName() << "is sending" << text;
}

/** Display tags in separate QComboBoxes. */
void TagEditor::displayTags()
{
	QList<QTableWidgetItem*> items = tagEditorWidget->selectedItems();

	// Information in the table is split into columns, using column index
	QMap<int, QStringList> datas;
	foreach (QTableWidgetItem *item, items) {

		// Load, feed and replace mechanism
		QStringList stringList = datas.value(item->column());
		stringList << item->text();
		datas.insert(item->column(), stringList);
	}

	// To avoid redondancy
	QMapIterator<int, QStringList> it(datas);
	while (it.hasNext()) {
		it.next();
		QStringList stringList = datas.value(it.key());
		stringList.removeDuplicates();
		stringList.sort();

		// Beware: there are no comboBox for every column in the edit area below the table
		QComboBox *combo = combos.value(it.key());
		if (combo) {
			combo->clear();
			combo->addItem(tr("(Keep)"));
			combo->addItem(tr("(Delete)"));
			combo->addItems(stringList);

			// No item: nothing is selected
			// 1 item: select this item
			// 2 or more: select (Keep)
			if (stringList.isEmpty()) {
				combo->setCurrentIndex(-1);
			} else if (stringList.count() == 1) {
				combo->setCurrentIndex(2);
			} else {
				combo->setCurrentIndex(0);
			}
		}
	}
}

void TagEditor::recordChanges(QTableWidgetItem *item)
{
	qDebug() << item->data(Qt::DisplayRole);
	if (!atLeastOneItemChanged) {
		atLeastOneItemChanged = true;
		saveChangesButton->setEnabled(true);
		cancelButton->setEnabled(true);
	}
	if (item->column() == 0) {
		// Filenames
		//bool b = filenames.value(item->row());
		//if (!b) {
		filenames.insert(item->row(), true);
		//}
	} else {
		// Tags
		//for (int i = 1; i < tagEditorWidget->columnCount(); i++) {
		//	QTableWidgetItem *i = tagEditorWidget->item(i, item->column());
		//
		//}
		tags.insert(item->row(), true);
	}
}

void TagEditor::toggleTagConverter(bool b)
{

	QPoint p = mapToGlobal(convertPushButton->pos());
	p.setX(p.x() - (tagConverter->width() - convertPushButton->width()) / 2);
	p.setY(p.y() + convertPushButton->height() + 5);
	tagConverter->move(p);
	tagConverter->setVisible(b);
}
