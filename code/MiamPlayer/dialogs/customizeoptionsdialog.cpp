#include "customizeoptionsdialog.h"

#include "mainwindow.h"
#include "pluginmanager.h"
#include "settings.h"
#include "shortcutwidget.h"

#include <QDir>
#include <QFileDialog>
#include <QLibraryInfo>
#include <QStandardPaths>

#include <QtDebug>

CustomizeOptionsDialog::CustomizeOptionsDialog(QWidget *parent) :
	QDialog(parent)
{
	setupUi(this);
	this->setWindowFlags(Qt::Tool);
	this->setModal(true);

	Settings *settings = Settings::getInstance();

	// First panel: library
	connect(radioButtonActivateDelegates, &QRadioButton::toggled, settings, &Settings::setDelegates);
	connect(pushButtonAddLocation, &QPushButton::clicked, this, &CustomizeOptionsDialog::openLibraryDialog);
	connect(pushButtonDeleteLocation, &QPushButton::clicked, this, &CustomizeOptionsDialog::deleteSelectedLocation);

	if (settings->isStarDelegates()) {
		radioButtonActivateDelegates->setChecked(true);
	} else {
		radioButtonDesactivateDelegates->setChecked(true);
	}

	QStringList locations = settings->musicLocations();
	if (locations.isEmpty()) {
		listWidgetMusicLocations->addItem(new QListWidgetItem(tr("Add some music locations here"), listWidgetMusicLocations));
	} else {
		foreach (QString path, locations) {
			listWidgetMusicLocations->addItem(new QListWidgetItem(QDir::toNativeSeparators(path), listWidgetMusicLocations));
		}
		listWidgetMusicLocations->setCurrentRow(0);
		pushButtonDeleteLocation->setEnabled(true);
	}

	// Second panel: languages
	connect(listViewLanguages, &QAbstractItemView::clicked, this, &CustomizeOptionsDialog::changeLanguage);

	QStandardItemModel *languageModel = new QStandardItemModel(this);
	listViewLanguages->setModel(languageModel);
	listViewLanguages->setGridSize(QSize(150, 64));
	listViewLanguages->setUniformItemSizes(true);

	QDir dir(":/languages");
	QStringList fileNames = dir.entryList();
	foreach (QString i, fileNames) {

		// If the language is available, then store it
		QFileInfo lang(":/translations/" + i);
		if (lang.exists()) {
			languages.insert(i, lang.filePath());
		}

		// Add a new country flag
		QStandardItem *languageItem = new QStandardItem(i);
		languageItem->setIcon(QIcon(dir.filePath(i)));
		languageModel->appendRow(languageItem);

		// Pick the right button in User Interface
		if (i == settings->language()) {
			listViewLanguages->setCurrentIndex(languageModel->indexFromItem(languageItem));
		}
	}

	// Third panel: shorcuts
	foreach(ShortcutWidget *shortcutWidget, findChildren<ShortcutWidget*>()) {
		connect(shortcutWidget, &ShortcutWidget::shortcutChanged, this, &CustomizeOptionsDialog::checkShortcut);
	}

	// Fourth panel: playback
	seekTimeSpinBox->setValue(settings->playbackSeekTime()/1000);
	connect(seekTimeSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), settings, &Settings::setPlaybackSeekTime);

	this->initCloseActionForPlaylists();
	connect(radioButtonAskAction, &QRadioButton::toggled, this, [=]() { settings->setPlaybackCloseAction(Settings::PL_AskUserForAction); });
	connect(radioButtonSavePlaylist, &QRadioButton::toggled, this, [=]() { settings->setPlaybackCloseAction(Settings::PL_SaveOnClose); });
	connect(radioButtonDiscardPlaylist, &QRadioButton::toggled, this, [=]() { settings->setPlaybackCloseAction(Settings::PL_DiscardOnClose); });

	if (settings->playbackKeepPlaylists()) {
		radioButtonKeepPlaylists->setChecked(true);
	} else {
		radioButtonClearPlaylists->setChecked(true);
	}
	connect(radioButtonKeepPlaylists, &QRadioButton::toggled, settings, &Settings::setPlaybackKeepPlaylists);

	if (settings->playbackRestorePlaylistsAtStartup()) {
		radioButtonRestorePlaylists->setChecked(true);
	} else {
		radioButtonDontRestorePlaylists->setChecked(true);
	}
	connect(radioButtonRestorePlaylists, &QRadioButton::toggled, settings, &Settings::setPlaybackRestorePlaylistsAtStartup);

	// Fifth panel: drag and drop
	this->initDragDropAction();
	//QRadioButton *radioButtonDD = this->findChild<QRadioButton*>(settings->dragAndDropBehaviour());
	//if (radioButtonDD) {
	//	radioButtonDD->setChecked(true);
	//}

	if (settings->copyTracksFromPlaylist()) {
		radioButtonDDCopyPlaylistTracks->setChecked(true);
	} else {
		radioButtonDDMovePlaylistTracks->setChecked(true);
	}

	connect(radioButtonDDOpenPopup, &QRadioButton::toggled, this, [=]() { settings->setDragDropAction(Settings::DD_OpenPopup); });
	connect(radioButtonDDAddToLibrary, &QRadioButton::toggled, this, [=]() { settings->setDragDropAction(Settings::DD_AddToLibrary); });
	connect(radioButtonDDAddToPlaylist, &QRadioButton::toggled, this, [=]() { settings->setDragDropAction(Settings::DD_AddToPlaylist); });
	connect(radioButtonDDCopyPlaylistTracks, &QRadioButton::toggled, settings, &Settings::setCopyTracksFromPlaylist);

	// Load the language of the application
	customTranslator.load(languages.value(Settings::getInstance()->language()));

	// Translate standard buttons (OK, Cancel, ...)
	defaultQtTranslator.load("qt_" + Settings::getInstance()->language(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));

	QApplication::installTranslator(&customTranslator);
	QApplication::installTranslator(&defaultQtTranslator);

	// Restore geometry
	this->restoreGeometry(settings->value("customizeOptionsDialogGeometry").toByteArray());
}

/** Is it necessary to redefined this from the UI class just for this init label? */
void CustomizeOptionsDialog::retranslateUi(CustomizeOptionsDialog *dialog)
{
	if (listWidgetMusicLocations->count() > 0 &&
			listWidgetMusicLocations->item(0)->text() == "Add some music locations here") {
		listWidgetMusicLocations->item(0)->setText(QApplication::translate(
			"CustomizeOptionsDialog", "Add some music locations here"));
	}
	// Retranslate the key if it's a special key like 'Space', 'Return' and so on...
	// And the modifiers 'Ctrl', 'Shift' and 'Alt'
	foreach(ShortcutWidget *shortcutWidget, findChildren<ShortcutWidget*>()) {

		QString source = QKeySequence(shortcutWidget->line()->key()).toString();
		QString translation = QApplication::translate("ShortcutLineEdit", source.toStdString().data());
		shortcutWidget->line()->setText(translation);

		// Item 0 is empty
		for (int i=1; i < shortcutWidget->modifiers()->count(); i++) {
			source = shortcutWidget->modifiers()->itemText(i);
			translation = QApplication::translate("ShortcutWidget", source.toStdString().data());
			/// bug with modifiers!
			shortcutWidget->modifiers()->setItemText(i, translation);
		}
	}

	Ui::CustomizeOptionsDialog::retranslateUi(dialog);
}

/** Redefined to add custom behaviour. */
void CustomizeOptionsDialog::closeEvent(QCloseEvent *e)
{
	QDialog::closeEvent(e);
	// Drive is scanned only when the popup is closed
	this->updateMusicLocations();
}

/** Adds a new music location in the library. */
void CustomizeOptionsDialog::addMusicLocation(const QString &musicLocation)
{
	int existingItem = -1;
	for (int i = 0; i < listWidgetMusicLocations->count(); i++) {
		QListWidgetItem *item = listWidgetMusicLocations->item(i);
		if (QString::compare(item->text(), QDir::toNativeSeparators(musicLocation)) == 0 ||
			QString::compare(item->text(), tr("Add some music locations here")) == 0) {
			existingItem = i;
			break;
		}
	}
	if (existingItem >= 0) {
		delete listWidgetMusicLocations->takeItem(existingItem);
	}
	listWidgetMusicLocations->addItem(new QListWidgetItem(QDir::toNativeSeparators(musicLocation), listWidgetMusicLocations));
	pushButtonDeleteLocation->setEnabled(true);
}

/** Adds a external music locations in the library (Drag & Drop). */
void CustomizeOptionsDialog::addMusicLocations(const QList<QDir> &dirs)
{
	foreach (QDir folder, dirs) {
		this->addMusicLocation(folder.absolutePath());
	}
	this->updateMusicLocations();
}

/** Change language at runtime. */
void CustomizeOptionsDialog::changeLanguage(QModelIndex index)
{
	QString lang = languages.value(index.data().toString());
	Settings *settings = Settings::getInstance();

	// If the language is successfully loaded, tells every widget that they need to be redisplayed
	if (!lang.isEmpty() && lang != settings->language() && customTranslator.load(lang)) {
		settings->setLanguage(index.data().toString());
		defaultQtTranslator.load("qt_" + lang, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
		QApplication::installTranslator(&customTranslator);
		/// TODO: reload plugin UI
		QApplication::installTranslator(&defaultQtTranslator);
	} else {
		labelStatusLanguage->setText(tr("No translation is available for this language :("));
	}
}

void CustomizeOptionsDialog::checkShortcut(ShortcutWidget *newShortcutAction, int typedKey)
{
	QMap<int, ShortcutWidget *> inverted;
	foreach(ShortcutWidget *sw, findChildren<ShortcutWidget*>()) {
		inverted.insertMulti(sw->key(), sw);
	}
	inverted.insertMulti(typedKey, newShortcutAction);

	Settings *settings = Settings::getInstance();
	MainWindow *mainWindow = qobject_cast<MainWindow *>(parent());
	QMapIterator<int, ShortcutWidget *> i(inverted);
	while (i.hasNext()) {
		i.next();
		if (i.key() != 0 && inverted.values(i.key()).size() > 2) {
			qDebug() << "parsing (1):";
			foreach (ShortcutWidget *sw, inverted.values(i.key())) {
				qDebug() << sw->objectName();
				sw->line()->setStyleSheet("QLineEdit { color: red }");
			}
		} else {
			mainWindow->bindShortcut(i.value()->objectName(), i.key());
			settings->setShortcut(i.value()->objectName(), i.key());
			inverted.value(i.key())->line()->setStyleSheet(QString());
		}
	}
}

/** Redefined to initialize theme from settings. */
void CustomizeOptionsDialog::open()
{
	foreach(MediaButton *b, parent()->findChildren<MediaButton*>()) {
		QPushButton *button = findChild<QPushButton*>(b->objectName());
		if (button) {
			button->setIcon(b->icon());
			button->setEnabled(b->isVisible());
			button->setChecked(b->isChecked());
		}
	}
	this->initCloseActionForPlaylists();
	this->initDragDropAction();
	retranslateUi(this);
	QDialog::open();
	this->activateWindow();
}

/** Delete a music location previously chosen by the user. */
void CustomizeOptionsDialog::deleteSelectedLocation()
{
	// If the user didn't click on an item before activating delete button
	int row = listWidgetMusicLocations->currentRow();
	if (row == -1) {
		row = 0;
	}

	Settings *settings = Settings::getInstance();
	if (!settings->musicLocations().isEmpty()) {
		delete listWidgetMusicLocations->takeItem(row);

		if (listWidgetMusicLocations->count() == 0) {
			pushButtonDeleteLocation->setEnabled(false);
			listWidgetMusicLocations->addItem(new QListWidgetItem(tr("Add some music locations here"), listWidgetMusicLocations));
		}
	}
}

void CustomizeOptionsDialog::initCloseActionForPlaylists()
{
	switch (Settings::getInstance()->playbackDefaultActionForClose()) {
	case Settings::PL_AskUserForAction:
		radioButtonAskAction->setChecked(true);
		break;
	case Settings::PL_SaveOnClose:
		radioButtonSavePlaylist->setChecked(true);
		break;
	case Settings::PL_DiscardOnClose:
		radioButtonDiscardPlaylist->setChecked(true);
		break;
	}
}

void CustomizeOptionsDialog::initDragDropAction()
{
	switch (Settings::getInstance()->dragDropAction()) {
	case Settings::DD_OpenPopup:
		radioButtonDDOpenPopup->setChecked(true);
		break;
	case Settings::DD_AddToLibrary:
		radioButtonDDAddToLibrary->setChecked(true);
		break;
	case Settings::DD_AddToPlaylist:
		radioButtonDDAddToPlaylist->setChecked(true);
		break;
	}
}

/** Open a dialog for letting the user to choose a music directory. */
void CustomizeOptionsDialog::openLibraryDialog()
{
	QString libraryPath = QFileDialog::getExistingDirectory(this, tr("Select a location of your music"),
		QStandardPaths::standardLocations(QStandardPaths::MusicLocation).first(), QFileDialog::ShowDirsOnly);
	if (!libraryPath.isEmpty()) {
		this->addMusicLocation(libraryPath);
	}
}

void CustomizeOptionsDialog::updateMusicLocations()
{
	Settings *settings = Settings::getInstance();
	QStringList savedLocations = settings->musicLocations();
	QStringList newLocations;
	for (int i = 0; i < listWidgetMusicLocations->count(); i++) {
		QString newLocation = listWidgetMusicLocations->item(i)->text();
		if (QString::compare(newLocation, tr("Add some music locations here")) != 0) {
			newLocations << newLocation;
		}
	}
	newLocations.sort();
	savedLocations.sort();

	bool musicLocationsAreIdenticals = true;
	if (savedLocations.size() == newLocations.size()) {
		for (int i = 0; i < savedLocations.count() && musicLocationsAreIdenticals; i++) {
			musicLocationsAreIdenticals = (QString::compare(savedLocations.at(i), newLocations.at(i)) == 0);
		}
	} else {
		musicLocationsAreIdenticals = false;
	}

	if (!musicLocationsAreIdenticals) {
		settings->setMusicLocations(newLocations);
		settings->sync();
		emit musicLocationsHaveChanged(newLocations.isEmpty());
	}
}
