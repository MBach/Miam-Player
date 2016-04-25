#include "tageditor.h"

#include <filehelper.h>
#include <settings.h>
#include <settingsprivate.h>
#include <treeview.h>

#include "pluginmanager.h"

#include <taglib/tfile.h>
#include <taglib/tpropertymap.h>

#include <QDir>
#include <QDirIterator>
#include <QDragEnterEvent>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>

#include <QtDebug>

QStringList TagEditor::genres = (QStringList() << "Blues" << "Classic Rock" << "Country" << "Dance" << "Disco" << "Funk" << "Grunge" << "Hip-Hop" << "Jazz"
	<< "Metal" << "New Age" << "Oldies" << "Other" << "Pop" << "R&B" << "Rap" << "Reggae" << "Rock" << "Techno"
	<< "Industrial" << "Alternative" << "Ska" << "Death Metal" << "Pranks" << "Soundtrack" << "Euro-Techno" << "Ambient"
	<< "Trip-Hop" << "Vocal" << "Jazz+Funk" << "Fusion" << "Trance" << "Classical" << "Instrumental" << "Acid" << "House"
	<< "Game" << "Sound Clip" << "Gospel" << "Noise" << "AlternRock" << "Bass" << "Soul" << "Punk" << "Space" << "Meditative"
	<< "Instrumental Pop" << "Instrumental Rock" << "Ethnic" << "Gothic" << "Darkwave" << "Techno-Industrial" << "Electronic"
	<< "Pop-Folk" << "Eurodance" << "Dream" << "Southern Rock" << "Comedy" << "Cult" << "Gangsta" << "Top 40"
	<< "Christian Rap" << "Pop/Funk" << "Jungle" << "Native American" << "Cabaret" << "New Wave" << "Psychedelic" << "Rave"
	<< "Showtunes" << "Trailer" << "Lo-Fi" << "Tribal" << "Acid Punk" << "Acid Jazz" << "Polka" << "Retro" << "Musical"
	<< "Rock & Roll" << "Hard Rock");

TagEditor::TagEditor(QWidget *parent)
	: AbstractView(nullptr, parent)
	, SelectedTracksModel()
{
	this->setWindowFlags(Qt::Window);
	setupUi(this);
	extensibleWidgetArea->setVisible(false);

	this->setAcceptDrops(true);

	tagEditorWidget->init();
	tagConverter = new TagConverter(convertPushButton, tagEditorWidget);

	_combos.insert(Miam::COL_Title, titleComboBox);
	_combos.insert(Miam::COL_Artist, artistComboBox);
	_combos.insert(Miam::COL_ArtistAlbum, artistAlbumComboBox);
	_combos.insert(Miam::COL_Album, albumComboBox);
	_combos.insert(Miam::COL_Track, trackComboBox);
	_combos.insert(Miam::COL_Disc, discComboBox);
	_combos.insert(Miam::COL_Year, yearComboBox);
	_combos.insert(Miam::COL_Genre, genreComboBox);
	_combos.insert(Miam::COL_Comment, commentComboBox);
	coverPathComboBox->setEditable(false);

	for (QComboBox *combo : _combos.values()) {
		combo->addItem(tr("(Keep)"));
		combo->addItem(tr("(Delete)"));
		combo->setCurrentIndex(-1);
		combo->setProperty("column", _combos.key(combo));
	}

	connect(saveChangesButton, &QPushButton::clicked, this, &TagEditor::commitChanges);
	connect(cancelButton, &QPushButton::clicked, this, &TagEditor::rollbackChanges);

	// General case: when one is selecting multiple items
	connect(tagEditorWidget, &QTableWidget::itemSelectionChanged, this, &TagEditor::displayTags);
	connect(tagEditorWidget, &QTableWidget::itemSelectionChanged, this, &TagEditor::displayCover);

	/// FIXME
	connect(albumCover, &AlbumCover::coverHasChanged, this, &TagEditor::replaceCover);
	connect(albumCover, &AlbumCover::aboutToApplyCoverToAll, this, &TagEditor::applyCoverToAll);

	albumCover->installEventFilter(this);
	tagEditorWidget->viewport()->installEventFilter(this);

	this->restoreGeometry(SettingsPrivate::instance()->value("tagEditorGeometry").toByteArray());
}

void TagEditor::addDirectory(const QDir &dir)
{
	QDirIterator it(dir, QDirIterator::Subdirectories);
	QStringList tracks;
	while (it.hasNext()) {
		QString entry = it.next();
		QFileInfo fileInfo(entry);
		if (fileInfo.isFile() && FileHelper::suffixes(FileHelper::ET_All).contains(fileInfo.suffix())) {
			tracks << fileInfo.absoluteFilePath();
		}
	}
	this->addTracks(tracks);
}

QPair<QString, QObjectList> TagEditor::extensionPoints() const
{
	QObjectList tagEditorObjectList;
	tagEditorObjectList << albumCover->contextMenu() << extensiblePushButtonArea << extensibleWidgetArea << tagEditorWidget;
	return qMakePair(metaObject()->className(), tagEditorObjectList);
}

QList<QUrl> TagEditor::selectedTracks()
{
	QList<QUrl> tracks;
	for (QModelIndex index : tagEditorWidget->selectionModel()->selectedRows(Miam::COL_Filename)) {
		tracks << QUrl::fromLocalFile(index.data(Qt::UserRole).toString());
	}
	return tracks;
}

void TagEditor::updateSelectedTracks()
{
	qDebug() << Q_FUNC_INFO << "Model has been updated, redraw selected tracks";
	//SqlDatabase().load();
}

bool TagEditor::viewProperty(Settings::ViewProperty vp) const
{
	switch (vp) {
	case Settings::VP_OwnWindow:
	case Settings::VP_HasTracksToDisplay:
		return true;
	default:
		return false;
	}
}

/** Redefined to be able to retransltate User Interface at runtime. */
void TagEditor::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange) {
		this->retranslateUi(this);
		tagConverter->retranslateUi(tagConverter);
	} else {
		QWidget::changeEvent(event);
	}
}

void TagEditor::closeEvent(QCloseEvent *event)
{
	SettingsPrivate::instance()->setValue("tagEditorGeometry", saveGeometry());
	AbstractView::closeEvent(event);
}

void TagEditor::dragEnterEvent(QDragEnterEvent *event)
{
	event->acceptProposedAction();
}

void TagEditor::dragMoveEvent(QDragMoveEvent *event)
{
	event->acceptProposedAction();
}

void TagEditor::dropEvent(QDropEvent *event)
{
	QObject *source = event->source();
	if (TreeView *view = qobject_cast<TreeView*>(source)) {
		view->openTagEditor();
	}
}

/** Redefined to filter context menu event for the cover album object. */
bool TagEditor::eventFilter(QObject *obj, QEvent *event)
{
	/// TEST
	if (obj == tagEditorWidget->viewport() && event->type() == QEvent::KeyRelease) {

	}

	if (obj == albumCover && event->type() == QEvent::ContextMenu) {
		return tagEditorWidget->selectedItems().isEmpty();
	} else if (obj == tagEditorWidget->viewport() && event->type() == QEvent::MouseButtonPress) {
		if (tagEditorWidget->selectionModel()->hasSelection()) {
			/// FIXME
			qDebug() << "selection";
			this->displayCover();
		}
		return QWidget::eventFilter(obj, event);
	} else {
		return QWidget::eventFilter(obj, event);
	}
}

void TagEditor::buildCache()
{
	qDebug() << Q_FUNC_INFO;
	tagEditorWidget->blockSignals(true);

	// Information in the table is split into columns, using column index
	// Column -> List of values ; [Col. Artist -> (AC/DC, Beatles, etc)]
	for (int col = 0; col < tagEditorWidget->columnCount(); col++) {
		for (int row = 0; row < tagEditorWidget->rowCount(); row++) {
			QSet<QString> stringList = _cacheData.value(col);
			auto item = tagEditorWidget->item(row, col);
			QFont f = item->font();
			f.setBold(false);
			item->setFont(f);
			stringList.insert(item->text());
			_cacheData.insert(col, stringList);
		}
	}

	tagEditorWidget->blockSignals(false);
}

void TagEditor::clearCovers(QMap<int, Cover*> &coversToRemove)
{
	QMutableMapIterator<int, Cover*> iterator(coversToRemove);
	while (iterator.hasNext()) {
		iterator.next();
		qDebug() << Q_FUNC_INFO << "clearCovers" << iterator.key() << (iterator.value() == nullptr);
		if (iterator.value() != nullptr) {
			delete iterator.value();
			iterator.value() = nullptr;
		}
	}
	coversToRemove.clear();
}

void TagEditor::replaceCover(Cover *newCover)
{
	for (QModelIndex index : tagEditorWidget->selectionModel()->selectedRows()) {
		Cover *previousCover = _covers.value(index.row());
		// It is sure that covers are different
		if (!(previousCover == nullptr || newCover == nullptr || qHash(previousCover->byteArray()) == qHash(newCover->byteArray()))) {
			newCover->setChanged(true);
			qDebug() << "TagEditor::replaceCover DELETE";
			delete previousCover;
		}
		_unsavedCovers.insert(index.row(), newCover);
	}
	saveChangesButton->setEnabled(true);
	cancelButton->setEnabled(true);
}

/** Splits tracks into columns to be able to edit metadatas. */
void TagEditor::addTracks(const QStringList &tracks)
{
	this->tagEditorWidget->setFocus();

	this->clear();
	this->buildCache();

	saveChangesButton->setEnabled(false);
	cancelButton->setEnabled(false);

	// It's possible to edit single items by double-clicking in the table
	// So, temporarily disconnect this signal
	disconnect(tagEditorWidget, &QTableWidget::itemChanged, this, &TagEditor::recordSingleItemChange);
	bool onlyOneAlbumIsSelected = tagEditorWidget->addItemsToEditor(tracks, _covers);
	/*qDebug() << Q_FUNC_INFO << "when adding tracks, " << covers.size() << "covers were found";
	for (int i = 0; i < covers.count(); i++) {
		Cover *c = covers.value(i);
		qDebug() << i << qHash(c->byteArray());
	}*/
	albumCover->setCoverForSingleAlbum(onlyOneAlbumIsSelected);

	connect(tagEditorWidget, &QTableWidget::itemChanged, this, &TagEditor::recordSingleItemChange);

	// Sort by path
	tagEditorWidget->setSortingEnabled(true);
	tagEditorWidget->sortItems(Miam::COL_Filename);
	tagEditorWidget->sortItems(Miam::COL_Path);
	tagEditorWidget->resizeColumnsToContents();
	tagEditorWidget->horizontalHeader()->setStretchLastSection(true);
}

/** Wrapper for addItemsToEditor. */
void TagEditor::addItemsToEditor(const QList<QUrl> &tracks)
{
	QStringList localFiles;
	for (QUrl url : tracks) {
		if (url.isLocalFile()) {
			localFiles.append(url.toLocalFile());
		}
	}
	this->addTracks(localFiles);
}

/** Clears all rows and comboboxes. */
void TagEditor::clear()
{
	//this->clearCovers(covers);
	//this->clearCovers(unsavedCovers);
	albumCover->resetCover();

	// Delete text contents, not the combobox itself
	for (QComboBox *combo : _combos.values()) {
		combo->clear();
	}
	tagEditorWidget->clear();
	_cacheData.clear();
}

void TagEditor::setViewProperty(Settings::ViewProperty vp, QVariant value)
{
	Q_UNUSED(vp)
	Q_UNUSED(value)
}

void TagEditor::applyCoverToAll(bool isForAll, Cover *cover)
{
	qDebug() << Q_FUNC_INFO << isForAll << cover;
	if (isForAll) {
		for (int row = 0; row < tagEditorWidget->rowCount(); row++) {
			Cover *c = _covers.value(row);
			if (c == nullptr) {
				qDebug() << Q_FUNC_INFO << "for row" << row << "cover is null";
				Cover *current = _unsavedCovers.value(row);
				if (current != nullptr) {
					qDebug() << Q_FUNC_INFO << "but there's already a cover waiting to be saved, deleting it and replacing it";
					delete current;
				}
				_unsavedCovers.insert(row, new Cover(cover->byteArray(), QString::fromUtf8(cover->mimeType().c_str())));
			} else {
				// Do not replace the cover for the caller
				if (c != cover) {
					qDebug() << Q_FUNC_INFO << row << cover;
					_unsavedCovers.insert(row, cover);
				}
			}
		}
	}
	saveChangesButton->setEnabled(true);
	cancelButton->setEnabled(true);
}

/** Saves all fields in the media. */
void TagEditor::commitChanges()
{
	qDebug() << Q_FUNC_INFO;
	// Create a subset of all modified tracks that needs to be rescanned by the model afterwards.
	QSet<int> tracksToRescan;

	QList<FileHelper*> _fhs;

	// Detect changes
	for (int row = 0; row < tagEditorWidget->rowCount(); row++) {
		// A physical and unique file per row
		QTableWidgetItem *itemFileName = tagEditorWidget->item(row, 0);
		QString absPath = itemFileName->data(Qt::UserRole).toString();
		FileHelper *fh = new FileHelper(absPath);
		_fhs.append(fh);
		bool trackWasModified = false;
		for (int col = 0; col < tagEditorWidget->columnCount(); col++) {

			// Check for every field if we have any changes
			QTableWidgetItem *item = tagEditorWidget->item(row, col);
			if (item && item->data(TagEditorTableWidget::MODIFIED).toBool()) {

				// If it has changed, we need to rename the file after setting meta-datas
				if (col == Miam::COL_Filename) {
					trackWasModified = true;
					qDebug() << absPath << "has been renamed";
				}

				// Replace the field by using a key stored in the header (one key per column)
				FileHelper::Field key = tagEditorWidget->horizontalHeaderItem(col)->data(TagEditorTableWidget::KEY).value<FileHelper::Field>();

				if (fh->file()->tag()) {
					trackWasModified = fh->insert(key, item->text()) || trackWasModified;
				} else {
					qDebug() << "no valid tag for this file";
				}
			}
		}

		Cover *previousCover = _covers.value(row);
		Cover *currentCover = _unsavedCovers.value(row);
		if ((previousCover == nullptr && currentCover != nullptr) || (previousCover != nullptr && currentCover == nullptr)) {
			fh->setCover(currentCover);
			trackWasModified = true;
		}

		// Save changes if at least one field was modified
		// Also, tell the model to rescan the file because the artist or the album might have changed:
		// The Tree structure in the Library could have been modified
		if (trackWasModified) {
			if (!fh->save()) {
				qDebug() << Q_FUNC_INFO << "tag wasn't saved :(";
			}
			tracksToRescan.insert(row);
		}
	}

	while (!_fhs.isEmpty()) {
		FileHelper *fh = _fhs.takeFirst();
		delete fh;
	}
	_fhs.clear();

	// Track has changed?
	if (!tracksToRescan.isEmpty()) {

		qDebug() << tracksToRescan.size() << "tracksToRescan.size()";
		QSetIterator<int> it(tracksToRescan);
		QStringList oldPaths, newPaths;
		while (it.hasNext()) {
			int row = it.next();

			QTableWidgetItem *filename = tagEditorWidget->item(row, Miam::COL_Filename);
			/// XXX: hard to find!
			QString oldFilepath = filename->data(Qt::UserRole).toString();
			if (filename->data(TagEditorTableWidget::MODIFIED).toBool()) {
				QTableWidgetItem *path = tagEditorWidget->item(row, Miam::COL_Path);
				QString newAbsPath = path->text() + QDir::separator() + filename->text();
				QFile f(oldFilepath);
				if (f.rename(newAbsPath)) {
					oldPaths << oldFilepath;
					newPaths << newAbsPath;
				}
				f.close();
			} else {
				oldPaths << oldFilepath;
				newPaths << QString();
			}
		}

		// Check if files are already in the library, and then update them
		if (!oldPaths.isEmpty()) {
			SqlDatabase db;
			connect(&db, &SqlDatabase::aboutToUpdateView, origin(), &AbstractView::modelReloadRequested);
			db.updateTracks(oldPaths, newPaths);
		}
	}

	// Reset buttons state
	saveChangesButton->setEnabled(false);
	cancelButton->setEnabled(false);

	tagEditorWidget->selectionModel()->clearSelection();
	this->buildCache();
	this->displayTags();
	tagEditorWidget->setFocus();
}

/** Displays a cover only if all the selected items have exactly the same cover. */
void TagEditor::displayCover()
{
	static Cover *_cover = nullptr;

	QMap<uint, Cover*> selectedCovers;
	QMap<int, QString> selectedAlbums;
	QString joinedTracks;
	// Extract only a subset of columns from the selected rows, in our case, only one column: displayed album name
	for (QModelIndex item : tagEditorWidget->selectionModel()->selectedRows(Miam::COL_Album)) {
		Cover *cover = nullptr;

		// Check if there's a cover in a temporary state (to allow rollback action)
		if (_unsavedCovers.value(item.row()) != nullptr) {
			cover = _unsavedCovers.value(item.row());
		} else {
			cover = _covers.value(item.row());
		}

		// Void items are excluded, so it will try display to something.
		// E.g.: if a cover is missing for one track but the whole album is selected.
		if (cover != nullptr && !cover->byteArray().isEmpty()) {
			selectedCovers.insert(qHash(cover->byteArray()), cover);
		}
		selectedAlbums.insert(item.row(), item.data().toString());
		QModelIndex track = item.sibling(item.row(), Miam::COL_Filename);
		joinedTracks += "\"" + track.data(Qt::UserRole).toString() + "\",";
	}
	joinedTracks.append("\"\"");

	// Fill the comboBox for the absolute path to the cover (if exists)
	SqlDatabase db;
	db.init();

	QSqlQuery coverPathQuery = db.exec("SELECT DISTINCT cover FROM tracks WHERE uri IN (" + joinedTracks + ")");
	QSet<QString> coversPath;
	while (coverPathQuery.next()) {
		coversPath << coverPathQuery.record().value(0).toString();
	}

	coverPathComboBox->clear();
	if (coversPath.size() > 1) {
		coverPathComboBox->addItem(tr("(Incompatible tracks selected)"));
		coverPathComboBox->addItems(coversPath.toList());
	} else if (coversPath.size() == 1) {
		coverPathComboBox->addItems(coversPath.toList());
	}

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
	if (tagEditorWidget->selectedItems().isEmpty()) {
		for (auto combo : _combos.values()) {
			combo->clear();
		}
		return;
	}

	QMap<int, QSet<QString>> selectedDatas;
	for (auto item : tagEditorWidget->selectedItems()) {
		QSet<QString> stringList = selectedDatas.value(item->column());
		stringList.insert(item->text());
		selectedDatas.insert(item->column(), stringList);
	}

	// To avoid redondancy, overwrite data for the same key
	QMapIterator<int, QSet<QString>> it(selectedDatas);
	while (it.hasNext()) {
		it.next();
		QSet<QString> stringList = selectedDatas.value(it.key());

		// Beware: there are no comboBox for every column in the edit area below the table
		QComboBox *combo = _combos.value(it.key());
		if (!combo) {
			continue;
		}

		disconnect(combo, &QComboBox::editTextChanged, this, &TagEditor::updateCells);
		combo->clear();

		// Special case for Genre, it's better to have them all in the combobox
		auto list = stringList.toList();
		std::sort(list.begin(), list.end());
		if (combo == genreComboBox) {
			combo->addItems(genres);
			for (QString genre : list) {
				if (!genres.contains(genre)) {
					combo->addItem(genre);
				}
			}
			combo->model()->sort(0);
		} else {
			combo->addItems(list);
		}

		int nextCurrentIndex = -1;

		// Multiple tracks selected but same value
		if (list.count() == 1) {
			if (combo == genreComboBox || combo == trackComboBox || combo == yearComboBox) {
				int result = combo->findText(list.first());
				nextCurrentIndex = result + 2;
			} else {
				nextCurrentIndex = 2;
			}
		} else {
			// Multiple tracks selected but for same attribute
			nextCurrentIndex = 0;
		}

		// Suggest data from the complete table
		if (combo != titleComboBox && combo != genreComboBox) {
			QSet<QString> allDatas = _cacheData.value(it.key());
			combo->setDuplicatesEnabled(false);
			for (QString c : allDatas) {
				if (combo->findText(c) == -1) {
					combo->addItem(c);
				}
			}
		}

		// Map the combobox object with the number of the column in the table to dynamically reflect changes
		// Arbitrarily adds the column number to the first item (Keep)
		combo->insertItem(0, tr("(Keep)"));
		combo->insertItem(1, tr("(Delete)"));
		combo->setCurrentIndex(nextCurrentIndex);

		connect(combo, &QComboBox::editTextChanged, this, &TagEditor::updateCells);
	}
}

void TagEditor::recordSingleItemChange(QTableWidgetItem *item)
{
	qDebug() << Q_FUNC_INFO;

	saveChangesButton->setEnabled(true);
	cancelButton->setEnabled(true);
	item->setData(TagEditorTableWidget::MODIFIED, true);
	QFont f(item->font());
	f.setBold(true);
	item->setFont(f);
}

/** Cancels all changes made by the user. */
void TagEditor::rollbackChanges()
{
	tagEditorWidget->blockSignals(true);
	tagEditorWidget->resetTable();

	this->replaceCover(nullptr);

	// Reset the unsaved cover list only
	this->clearCovers(_unsavedCovers);

	tagEditorWidget->blockSignals(false);
	// qDebug() << "rollbackChanges";

	// Then, reload info a second time
	this->displayCover();
	this->displayTags();

	saveChangesButton->setEnabled(false);
	cancelButton->setEnabled(false);
}

/** When one is changing a field, updates all rows in the table (the Artist for example). */
void TagEditor::updateCells(QString text)
{
	QComboBox *combo = findChild<QComboBox*>(sender()->objectName());
	int column = combo->property("column").toInt();

	combo->blockSignals(true);

	// Special behaviour for "Keep" and "Delete" items
	switch (combo->currentIndex()) {
	// "(Keep)" item
	case 0: {
		QModelIndexList list = tagEditorWidget->selectionModel()->selectedRows(column);
		for (int i = 0; i < list.size(); i++) {
			QModelIndex index = list.at(i);
			// Fill the table with one item per combobox
			if (combo == titleComboBox || combo == trackComboBox) {
				tagEditorWidget->item(index.row(), column)->setText(combo->itemText(2 + i));
			// For unique attribute like "Artist" or "year" copy-paste this item to every cells in the table
			} else if (combo == artistComboBox || combo == artistAlbumComboBox || combo == albumComboBox || combo == yearComboBox
					   || combo == discComboBox || combo == genreComboBox || combo == commentComboBox) {
				tagEditorWidget->item(index.row(), column)->setText(combo->itemText(2));
			}
			auto item = tagEditorWidget->item(index.row(), column);
			item->setData(TagEditorTableWidget::MODIFIED, true);
			QFont f(item->font());
			qDebug() << Q_FUNC_INFO << column;
			f.setBold(true);
			item->setFont(f);
		}
		break;
	}
	// "(Delete)" item
	case 1:
		tagEditorWidget->updateColumnData(column, "");
		break;
	// A regular item
	default:
		tagEditorWidget->updateColumnData(column, text);
		break;
	}
	//saveChangesButton->setEnabled(true);
	//cancelButton->setEnabled(true);
	tagEditorWidget->resizeColumnToContents(column);

	combo->blockSignals(false);
}
