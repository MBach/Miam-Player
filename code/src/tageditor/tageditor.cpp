#include "tageditor.h"
#include "library/libraryitem.h"
#include "settings.h"

#include <QtDebug>

#include <QFile>
#include <QGraphicsPixmapItem>

#include <fileref.h>
#include <tag.h>
#include <tpropertymap.h>

using namespace TagLib;

TagEditor::TagEditor(QWidget *parent) :
	QWidget(parent), atLeastOneItemChanged(false)
{
	setupUi(this);
	tagEditorWidget->init();

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
	//tags.insert(tagEditorWidget->rowCount() - 1, false);
}

void TagEditor::afterAddingItems()
{
	// It's possible to edit single items by double-clicking in the table
	connect(tagEditorWidget, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(recordChanges(QTableWidgetItem*)));
	tagEditorWidget->resizeColumnsToContents();
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
	QWidget::close();
}

void TagEditor::commitChanges()
{
	//bool libraryNodesNeedToBeRebuild = false;

	// Detect changes
	for (int i = 0; i < tagEditorWidget->rowCount(); i++) {
		// A physical file per row
		FileRef fileRef = tagEditorWidget->trackList().at(i);
		bool trackWasModified = false;
		for (int j = 0; j < tagEditorWidget->columnCount(); j++) {

			// Check for every field if we have any changes
			QTableWidgetItem *item = tagEditorWidget->item(i, j);
			if (item->data(TagEditorTableWidget::MODIFIED).toBool()) {

				// Replace the field by using a key stored in the header (one key per column)
				QString key = tagEditorWidget->horizontalHeaderItem(j)->data(TagEditorTableWidget::KEY).toString();
				PropertyMap pm = fileRef.tag()->properties();
				/// XXX: warning !
				/// Detect when one in changing ARTIST or ALBUM tag to rebuild the tree on the left!
				if (key == "ALBUM" || "ARTIST") {
					//libraryNodesNeedToBeRebuild = true;
				}
				bool b = pm.replace(String(key.toStdString()), String(item->text().toStdString()));
				if (b) {
					fileRef.tag()->setProperties(pm);
					trackWasModified = true;
				}
			}
		}
		// Save changes if at least one field was modified
		if (trackWasModified) {
			fileRef.save();
		}
	}
	/*if (libraryNodesNeedToBeRebuild) {
		emit rebuildNodes();
	}*/

	//QMapIterator<int, bool> f(filenames);
	// Apply new filenames
	/// TODO
	/*while (f.hasNext()) {
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
	}*/

	saveChangesButton->setEnabled(false);
	cancelButton->setEnabled(false);
}

/** Display tags in separate QComboBoxes. */
void TagEditor::displayTags()
{
	// Can be multiples rows
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

		// Beware: there are no comboBox for every column in the edit area below the table
		QComboBox *combo = combos.value(it.key());
		if (combo) {

			disconnect(combo, SIGNAL(editTextChanged(QString)), this, SLOT(updateCells(QString)));
			combo->clear();
			combo->addItem(tr("(Keep)"));
			// Map the combobox object with the number of the column in the table to dynamically reflect changes
			// Arbitrarily adds the column number to the first item (Keep)
			combo->setItemData(0, combos.key(combo), Qt::UserRole+1);
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
			connect(combo, SIGNAL(editTextChanged(QString)), this, SLOT(updateCells(QString)));
		}
	}
}

void TagEditor::recordChanges(QTableWidgetItem *item)
{
	if (!atLeastOneItemChanged) {
		atLeastOneItemChanged = true;
		saveChangesButton->setEnabled(true);
		cancelButton->setEnabled(true);
	}
	if (item->column() == 0) {
		// Filenames
		filenames.insert(item->row(), true);
	} else {
		// Tags
		//tags.insert(item->row(), true);
	}
}

void TagEditor::rollbackChanges()
{
	tagEditorWidget->resetTable();
	saveChangesButton->setEnabled(false);
	cancelButton->setEnabled(false);
	atLeastOneItemChanged = false;
}

void TagEditor::toggleTagConverter(bool b)
{
	QPoint p = mapToGlobal(convertPushButton->pos());
	p.setX(p.x() - (tagConverter->width() - convertPushButton->width()) / 2);
	p.setY(p.y() + convertPushButton->height() + 5);
	tagConverter->move(p);
	tagConverter->setVisible(b);
}

/** When one is changing a field, updates all rows in the table (the Artist for example). */
void TagEditor::updateCells(QString text)
{
	QComboBox *combo = findChild<QComboBox*>(sender()->objectName());
	QVariant v = combo->itemData(0, Qt::UserRole+1);
	int idxColumnInTable = v.toInt();

	disconnect(combo, SIGNAL(editTextChanged(QString)), this, SLOT(updateCells(QString)));

	// Special behaviour for "Keep" and "Delete" items
	// Keep
	if (combo->currentIndex() == 0) {

		QModelIndexList list = tagEditorWidget->selectionModel()->selectedRows(idxColumnInTable);
		for (int i = 0; i < list.size(); i++) {
			QModelIndex index = list.at(i);
			// Fill the table with one item per combobox
			if (combo == titleComboBox || combo == trackComboBox) {
				tagEditorWidget->item(index.row(), idxColumnInTable)->setText(combo->itemText(2 + i));
			// For unique attribute like "Artist" or "year" copy-paste this item to every cells in the table
			} else if (combo == artistComboBox || combo == artistAlbumComboBox || combo == albumComboBox || combo == yearComboBox
					   || combo == discComboBox || combo == genreComboBox || combo == commentComboBox) {
				tagEditorWidget->item(index.row(), idxColumnInTable)->setText(combo->itemText(2));
			}
			tagEditorWidget->item(index.row(), idxColumnInTable)->setData(TagEditorTableWidget::MODIFIED, true);
		}

	// Delete
	} else if (combo->currentIndex() == 1) {
		tagEditorWidget->updateColumnData(idxColumnInTable, "");
	// A regular item
	} else {
		tagEditorWidget->updateColumnData(idxColumnInTable, text);
	}

	connect(combo, SIGNAL(editTextChanged(QString)), this, SLOT(updateCells(QString)));
}
