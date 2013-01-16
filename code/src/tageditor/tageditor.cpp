#include "tageditor.h"
#include "filehelper.h"
#include "library/libraryitem.h"
#include "settings.h"

#include <tpropertymap.h>
#include <MediaSource>
#include <QDir>

#include <QtDebug>

using namespace TagLib;

QStringList TagEditor::genres = (QStringList() << "Blues" << "Classic Rock" << "Country" << "Dance" << "Disco" << "Funk" << "Grunge" << "Hip-Hop" << "Jazz"
	<< "Metal" << "New Age" << "Oldies" << "Other" << "Pop" << "R&B" << "Rap" << "Reggae" << "Rock" << "Techno"
	<< "Industrial" << "Alternative" << "Ska" << "Death Metal" << "Pranks" << "Soundtrack" << "Euro-Techno" << "Ambient"
	<< "Trip-Hop" << "Vocal" << "Jazz+Funk" << "Fusion" << "Trance" << "Classical" << "Instrumental" << "Acid" << "House"
	<< "Game" << "Sound Clip" << "Gospel" << "Noise" << "AlternRock" << "Bass" << "Soul" << "Punk" << "Space" << "Meditative"
	<< "Instrumental Pop" << "Instrumental Rock" << "Ethnic" << "Gothic" << "Darkwave" << "Techno-Industrial" << "Electronic"
	<< "Pop-Folk" << "Eurodance" << "Dream" << "Southern Rock" << "Comedy" << "Cult" << "Gangsta" << "Top 40"
	<< "Christian Rap" << "Pop/Funk" << "Jungle" << "Native American" << "Cabaret" << "New Wave" << "Psychadelic" << "Rave"
	<< "Showtunes" << "Trailer" << "Lo-Fi" << "Tribal" << "Acid Punk" << "Acid Jazz" << "Polka" << "Retro" << "Musical"
	<< "Rock & Roll" << "Hard Rock");

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

/** Clear all rows and comboboxes. */
void TagEditor::clear()
{
	// Delete text contents, not the combobox itself
	foreach (QComboBox *combo, combos.values()) {
		combo->clear();
	}
	tagEditorWidget->clear();
}

/** Split tracks into columns to be able to edit metadatas. */
void TagEditor::addItemsToEditor(const QList<QPersistentModelIndex> &indexes)
{
	this->clear();
	saveChangesButton->setEnabled(false);
	cancelButton->setEnabled(false);

	// It's possible to edit single items by double-clicking in the table
	// So, temporarily disconnect this signal
	disconnect(tagEditorWidget, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(recordSingleItemChange(QTableWidgetItem*)));
	tagEditorWidget->addItemsToEditor(indexes);
	connect(tagEditorWidget, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(recordSingleItemChange(QTableWidgetItem*)));

	// Sort by path
	tagEditorWidget->setSortingEnabled(true);
	tagEditorWidget->sortItems(0);
	tagEditorWidget->sortItems(1);
	tagEditorWidget->resizeColumnsToContents();
}

/** Close this Widget and tells its parent to switch views. */
void TagEditor::close()
{
	emit closeTagEditor(false);
	saveChangesButton->setEnabled(false);
	cancelButton->setEnabled(false);
	atLeastOneItemChanged = false;
	this->clear();
	QWidget::close();
}

/** Save all fields in the media. */
void TagEditor::commitChanges()
{
	// Create a subset of all modified tracks that needs to be rescanned by the model afterwards.
	QList<QPersistentModelIndex> tracksToRescan;

	// Detect changes
	for (int i = 0; i < tagEditorWidget->rowCount(); i++) {
		// A physical and unique file per row
		QTableWidgetItem *itemFileName = tagEditorWidget->item(i, 0);
		QTableWidgetItem *itemPath = tagEditorWidget->item(i, 1);
		QString absPath(itemPath->text() + QDir::separator() + itemFileName->text());
		FileRef file(QFile::encodeName(absPath).data());
		FileHelper fh(file, itemFileName->data(LibraryItem::SUFFIX).toInt());
		bool trackWasModified = false;
		for (int j = 2; j < tagEditorWidget->columnCount(); j++) {

			// Check for every field if we have any changes
			QTableWidgetItem *item = tagEditorWidget->item(i, j);

			if (item && item->data(TagEditorTableWidget::MODIFIED).toBool()) {

				// Replace the field by using a key stored in the header (one key per column)
				QString key = tagEditorWidget->horizontalHeaderItem(j)->data(TagEditorTableWidget::KEY).toString();
				if (file.tag()) {
					PropertyMap pm = file.tag()->properties();

					// The map doesn't always contains all keys, like ArtistAlbum (not standard)
					if (pm.contains(key.toStdString())) {
						bool b = pm.replace(key.toStdString(), String(item->text().toStdString()));
						if (b) {
							file.tag()->setProperties(pm);
							if (file.tag()) {
								trackWasModified = true;
							}
						}
					} else {
						trackWasModified = fh.insert(key, item->text());
					}
				} else {
					qDebug() << "no valid tag for this file";
				}
			}
		}
		// Save changes if at least one field was modified
		// Also, tell the model to rescan the file because the artist or the album might have changed:
		// The Tree structure could be modified
		if (trackWasModified) {
			bool b = fh.save();
			if (!b) {
				qDebug() << "tag wasn't saved :(";
			}
			tracksToRescan.append(tagEditorWidget->index(absPath));
		}
	}

	if (!tracksToRescan.isEmpty()) {
		emit rebuildTreeView(tracksToRescan);
	}

	saveChangesButton->setEnabled(false);
	cancelButton->setEnabled(false);
	atLeastOneItemChanged = false;
}

/** Display tags in separate QComboBoxes. */
void TagEditor::displayTags()
{
	// Information in the table is split into columns, using column index
	QMap<int, QStringList> datas;
	QMap<int, QVariant> covers;

	int lastCol = tagEditorWidget->columnCount() - 1;

	// Can be multiples rows
	QModelIndexList items = tagEditorWidget->selectionModel()->selectedIndexes();
	foreach (QModelIndex item, items) {
		// Load, feed and replace mechanism
		// For the last column (which is the cover), use QPixmap instead
		if (item.column() == lastCol) {
			covers.insert(item.row(), item.data(Qt::EditRole));
		} else {
			QStringList stringList = datas.value(item.column());
			stringList << item.data().toString();
			datas.insert(item.column(), stringList);
		}
	}

	/// XXX
	if (covers.size() == 1) {
		QMapIterator<int, QVariant> it(covers);
		while (it.hasNext()) {
			it.next();
			labelCover->displayFromAttachedPicture(it.value());
		}
	}

	// To avoid redondancy, overwrite data for the same key
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
			combo->insertItem(0, tr("(Keep)"));
			// Map the combobox object with the number of the column in the table to dynamically reflect changes
			// Arbitrarily adds the column number to the first item (Keep)
			combo->setItemData(0, combos.key(combo), Qt::UserRole+1);
			combo->insertItem(1, tr("(Delete)"));

			// Special case for Genre, it's better to have them all in the combobox
			if (combo == genreComboBox) {
				combo->addItems(genres);
				foreach (QString genre, stringList) {
					if (!genres.contains(genre)) {
						combo->addItem(genre);
					}
				}
			} else {
				combo->addItems(stringList);
			}

			// Special case for Tracknumber and Year: it's better to have a numerical order
			if (combo == genreComboBox || combo == trackComboBox || combo == yearComboBox) {
				combo->model()->sort(0);
			}

			// No item: nothing is selected
			// 1 item: select this item
			// 2 or more: select (Keep)
			if (stringList.isEmpty()) {
				combo->setCurrentIndex(-1);
			} else if (stringList.count() == 1) {
				if (combo == genreComboBox || combo == trackComboBox || combo == yearComboBox) {
					int result = combo->findText(stringList.first());
					combo->setCurrentIndex(result);
				} else {
					combo->setCurrentIndex(2);
				}
			} else {
				combo->setCurrentIndex(0);
			}
			connect(combo, SIGNAL(editTextChanged(QString)), this, SLOT(updateCells(QString)));
		}
	}
}

void TagEditor::recordSingleItemChange(QTableWidgetItem *item)
{
	saveChangesButton->setEnabled(true);
	cancelButton->setEnabled(true);
	item->setData(TagEditorTableWidget::MODIFIED, true);
}

/** Cancel all changes made by the user. */
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
	saveChangesButton->setEnabled(true);
	cancelButton->setEnabled(true);

	connect(combo, SIGNAL(editTextChanged(QString)), this, SLOT(updateCells(QString)));
}
