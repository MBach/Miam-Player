#include "tageditor.h"
#include "filehelper.h"
#include "settings.h"
#include "pluginmanager.h"
#include "model/librarysqlmodel.h"

#include <3rdparty/taglib/tfile.h>
#include <3rdparty/taglib/tpropertymap.h>
#include <QDir>

#include <QtDebug>

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
	QWidget(parent), SelectedTracksModel(), _sqlModel(NULL)
{
	setupUi(this);

	tagConverter = new TagConverter(this);
	tagEditorWidget->init();

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
	coverPathComboBox->setEditable(false);

	foreach (QComboBox *combo, combos.values()) {
		combo->addItem(tr("(Keep)"));
		combo->addItem(tr("(Delete)"));
		combo->setCurrentIndex(-1);
	}
	// Quit this widget when a request was send from this button
	connect(closeTagEditorButton, &QPushButton::clicked, this, &TagEditor::close);

	connect(saveChangesButton, &QPushButton::clicked, this, &TagEditor::commitChanges);
	connect(cancelButton, &QPushButton::clicked, this, &TagEditor::rollbackChanges);

	// General case: when one is selecting multiple items
	connect(tagEditorWidget, &QTableWidget::itemSelectionChanged, this, &TagEditor::displayCover);
	connect(tagEditorWidget, &QTableWidget::itemSelectionChanged, this, &TagEditor::displayTags);

	// Open the TagConverter to help tagging from Tag to File, or vice-versa
	connect(convertPushButton, &QPushButton::toggled, this, &TagEditor::toggleTagConverter);

	connect(albumCover, &AlbumCover::coverHasChanged, this, &TagEditor::replaceCover);
	connect(albumCover, &AlbumCover::aboutToApplyCoverToAll, this, &TagEditor::applyCoverToAll);

	// The context menu in the area displaying a cover can be extended by third party
	QObjectList objectsToExtend = QObjectList() << albumCover->contextMenu() << this;
	PluginManager::getInstance()->registerExtensionPoint(this->metaObject()->className(), objectsToExtend);

	albumCover->installEventFilter(this);
}

void TagEditor::init(LibrarySqlModel *sqlModel)
{
	_sqlModel = sqlModel;
}

QStringList TagEditor::selectedTracks()
{
	QStringList tracks;
	foreach (QModelIndex index, tagEditorWidget->selectionModel()->selectedRows(TagEditorTableWidget::COL_Filename)) {
		tracks << index.data(Qt::UserRole).toString();
	}
	return tracks;
}

void TagEditor::updateSelectedTracks()
{
	qDebug() << "TagEditor: model has been updated, redraw selected tracks";
	_sqlModel->load();
}

/** Redefined to filter context menu event for the cover album object. */
bool TagEditor::eventFilter(QObject *obj, QEvent *event)
{
	if (obj == albumCover && event->type() == QEvent::ContextMenu) {
		return tagEditorWidget->selectedItems().isEmpty();
	} else {
		return QWidget::eventFilter(obj, event);
	}
}

void TagEditor::clearCovers(QMap<int, Cover*> &coversToRemove)
{
	QMutableMapIterator<int, Cover*> iterator(coversToRemove);
	while (iterator.hasNext()) {
		iterator.next();
		qDebug() << "clearCovers" << iterator.key() << (iterator.value() == NULL);
		if (iterator.value() != NULL) {
			delete iterator.value();
			iterator.value() = NULL;
		}
	}
	coversToRemove.clear();
}

void TagEditor::replaceCover(Cover *newCover)
{
	foreach (QModelIndex index, tagEditorWidget->selectionModel()->selectedRows()) {
		Cover *previousCover = covers.value(index.row());
		// It is sure that covers are different
		if (!(previousCover == NULL || newCover == NULL || qHash(previousCover->byteArray()) == qHash(newCover->byteArray()))) {
			newCover->setChanged(true);
			qDebug() << "TagEditor::replaceCover DELETE";
			delete previousCover;
		}
		unsavedCovers.insert(index.row(), newCover);
	}
	saveChangesButton->setEnabled(true);
	cancelButton->setEnabled(true);
}

/** Splits tracks into columns to be able to edit metadatas. */
void TagEditor::addItemsToEditor(const QModelIndexList &sourceIndexes, const QStringList &tracks)
{
	this->tagEditorWidget->setFocus();
	_sourceIndexes = sourceIndexes;

	this->clear();
	saveChangesButton->setEnabled(false);
	cancelButton->setEnabled(false);

	// It's possible to edit single items by double-clicking in the table
	// So, temporarily disconnect this signal
	disconnect(tagEditorWidget, &QTableWidget::itemChanged, this, &TagEditor::recordSingleItemChange);
	//qDebug() << "TagEditor::addItemsToEditor" << tracks;
	bool onlyOneAlbumIsSelected = tagEditorWidget->addItemsToEditor(tracks, covers);
	albumCover->setCoverForUniqueAlbum(onlyOneAlbumIsSelected);
	connect(tagEditorWidget, &QTableWidget::itemChanged, this, &TagEditor::recordSingleItemChange);

	// Sort by path
	tagEditorWidget->setSortingEnabled(true);
	tagEditorWidget->sortItems(TagEditorTableWidget::COL_Filename);
	tagEditorWidget->sortItems(TagEditorTableWidget::COL_Path);
	tagEditorWidget->resizeColumnsToContents();
}

/** Wrapper for addItemsToEditor. */
void TagEditor::addUrlsToEditor(const QList<QUrl> &tracks)
{
	QStringList localFiles;
	foreach (QUrl url, tracks) {
		if (url.isLocalFile()) {
			localFiles.append(url.toLocalFile());
		}
	}
	this->addItemsToEditor(QModelIndexList(), localFiles);
}

/** Clears all rows and comboboxes. */
void TagEditor::clear()
{
	this->clearCovers(covers);
	this->clearCovers(unsavedCovers);

	albumCover->resetCover();

	// Delete text contents, not the combobox itself
	foreach (QComboBox *combo, combos.values()) {
		combo->clear();
	}
	tagEditorWidget->clear();
}

void TagEditor::applyCoverToAll(bool isForAll, Cover *cover)
{
	if (isForAll) {
		for (int row = 0; row < tagEditorWidget->rowCount(); row++) {
			Cover *c = covers.value(row);
			if (c == NULL) {
				unsavedCovers.insert(row, new Cover(cover->byteArray(), QString::fromUtf8(cover->mimeType().c_str())));
			} else {
				// Do not replace the cover for the caller
				if (c != cover) {
					unsavedCovers.insert(row, cover);
				}
			}
		}
	}

	//saveChangesButton->setEnabled(true);
	cancelButton->setEnabled(true);
	qDebug() << "applyCoverToAll" << isForAll;
}

/** Closes this Widget and tells its parent to switch views. */
void TagEditor::close()
{
	emit aboutToCloseTagEditor();
	saveChangesButton->setEnabled(false);
	cancelButton->setEnabled(false);
	this->clear();
}

/** Saves all fields in the media. */
void TagEditor::commitChanges()
{
	// Create a subset of all modified tracks that needs to be rescanned by the model afterwards.
	QList<QModelIndex> tracksToRescan;
	QStringList tracksToRescan2;

	// Detect changes
	for (int i = 0; i < tagEditorWidget->rowCount(); i++) {
		// A physical and unique file per row
		QTableWidgetItem *itemFileName = tagEditorWidget->item(i, 0);
		QString absPath = itemFileName->data(Qt::UserRole).toString();
		FileHelper fh(absPath);
		bool trackWasModified = false;
		for (int j = 2; j < tagEditorWidget->columnCount(); j++) {

			// Check for every field if we have any changes
			QTableWidgetItem *item = tagEditorWidget->item(i, j);
			if (item && item->data(TagEditorTableWidget::MODIFIED).toBool()) {

				// Replace the field by using a key stored in the header (one key per column)
				QString key = tagEditorWidget->horizontalHeaderItem(j)->data(TagEditorTableWidget::KEY).toString();
				if (fh.file()->tag()) {
					TagLib::PropertyMap pm = fh.file()->tag()->properties();

					// The map doesn't always contain all keys, like ArtistAlbum (not standard)
					if (pm.contains(key.toStdString())) {
						bool b = pm.replace(key.toStdString(), TagLib::String(item->text().toStdString()));
						if (b) {
							fh.file()->tag()->setProperties(pm);
							if (fh.file()->tag()) {
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

		Cover *cover = unsavedCovers.value(i);
		if (cover == NULL || (cover != NULL && cover->hasChanged())) {
			qDebug() << "setCover(" << i << ")";
			fh.setCover(cover);
			trackWasModified = true;
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
			tracksToRescan2 << absPath;
		}
	}

	if (!tracksToRescan.isEmpty()) {
		/// TODO: possibility to rename file and path!
		_sqlModel->updateLibrary(tracksToRescan2);
	}

	// Reset buttons state
	saveChangesButton->setEnabled(false);
	cancelButton->setEnabled(false);
}

#include <QSqlQuery>
#include <QSqlRecord>

/** Displays a cover only if all the selected items have exactly the same cover. */
void TagEditor::displayCover()
{
	static Cover *_cover = NULL;

	QMap<int, Cover*> selectedCovers;
	QMap<int, QString> selectedAlbums;
	// Extract only a subset of columns from the selected rows, in our case, only one column: displayed album name
	foreach (QModelIndex item, tagEditorWidget->selectionModel()->selectedRows(TagEditorTableWidget::COL_Album)) {
		Cover *cover = NULL;
		// Check if there's a cover in a temporary state (to allow rollback action)
		if (unsavedCovers.value(item.row()) != NULL) {
			cover = unsavedCovers.value(item.row());
		} else {
			cover = covers.value(item.row());
		}

		// Void items are excluded, so it will try display to something.
		// E.g.: if a cover is missing for one track but the whole album is selected.
		if (cover != NULL && !cover->byteArray().isEmpty()) {
			selectedCovers.insert(item.row(), cover);
		}
		selectedAlbums.insert(item.row(), item.data().toString());
	}

	// Fill the comboBox for the absolute path to the cover (if exists)
	_sqlModel->database().open();
	QSet<QString> coversPath;
	foreach (QString track, selectedTracks()) {
		QSqlQuery coverPathQuery(_sqlModel->database());
		coverPathQuery.prepare("SELECT coverAbsPath FROM tracks WHERE absPath = ?");
		coverPathQuery.addBindValue(track);
		if (coverPathQuery.exec()) {
			coverPathQuery.next();
			coversPath << coverPathQuery.record().value(0).toString();
		}
	}
	coverPathComboBox->clear();
	if (coversPath.size() > 1) {
		coverPathComboBox->addItem(tr("(Incompatible tracks selected)"));
		coverPathComboBox->addItems(coversPath.toList());
	} else if (coversPath.size() == 1) {
		coverPathComboBox->addItems(coversPath.toList());
	}
	_sqlModel->database().close();

	// Beware: if a cover is shared between multiple albums, only the first album name will appear in the context menu.
	if (selectedCovers.size() == 1) {
		_cover = selectedCovers.values().first();
		albumCover->setCover(_cover);
	} else if (coversPath.size() == 1) {
		delete _cover;
		_cover = new Cover(coversPath.toList().first());
		albumCover->setCover(_cover);
	} else {
		albumCover->resetCover();
	}
	if (selectedAlbums.size() == 1) {
		albumCover->setAlbum(selectedAlbums.values().first());
	}
}

/** Displays tags in separate QComboBoxes. */
void TagEditor::displayTags()
{
	// Information in the table is split into columns, using column index
	QMap<int, QStringList> datas;
	foreach (QTableWidgetItem *item, tagEditorWidget->selectedItems()) {
		QStringList stringList = datas.value(item->column());
		stringList << item->text();
		datas.insert(item->column(), stringList);
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

			disconnect(combo, &QComboBox::editTextChanged, this, &TagEditor::updateCells);
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
			connect(combo, &QComboBox::editTextChanged, this, &TagEditor::updateCells);
		}
	}
}

void TagEditor::recordSingleItemChange(QTableWidgetItem *item)
{
	saveChangesButton->setEnabled(true);
	cancelButton->setEnabled(true);
	item->setData(TagEditorTableWidget::MODIFIED, true);
}

/** Cancels all changes made by the user. */
void TagEditor::rollbackChanges()
{
	tagEditorWidget->resetTable();
	saveChangesButton->setEnabled(false);
	cancelButton->setEnabled(false);

	qDebug() << "rollbackChanges";
	tagEditorWidget->blockSignals(true);
	this->replaceCover(NULL);

	// Reset the unsaved cover list only
	this->clearCovers(unsavedCovers);

	tagEditorWidget->blockSignals(false);
	qDebug() << "rollbackChanges";

	// Then, reload info a second time
	//this->displayCover();
	//this->displayTags();
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

	combo->blockSignals(true);

	// Special behaviour for "Keep" and "Delete" items
	switch (combo->currentIndex()) {
	// "(Keep)" item
	case 0: {
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
		break;
	}
	// "(Delete)" item
	case 1:
		tagEditorWidget->updateColumnData(idxColumnInTable, "");
		break;
	// A regular item
	default:
		tagEditorWidget->updateColumnData(idxColumnInTable, text);
		break;
	}
	saveChangesButton->setEnabled(true);
	cancelButton->setEnabled(true);

	combo->blockSignals(false);
}
