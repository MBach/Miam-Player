#include "customizeoptionsdialog.h"

#include "mainwindow.h"
#include "settings.h"

#include <QDir>
#include <QFileDialog>
#include <QStandardPaths>

#include <QtDebug>

#include <QLibraryInfo>

#include "shortcutwidget.h"

CustomizeOptionsDialog::CustomizeOptionsDialog(QWidget *parent) :
	QDialog(parent), musicLocationsChanged(false)
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

	QList<QVariant> locations = settings->musicLocations();
	if (locations.isEmpty()) {
		listWidgetMusicLocations->addItem(new QListWidgetItem("Add some music locations here"));
	} else {
		for (int i=0; i<locations.count(); i++) {
			QString convertedPath = QDir::toNativeSeparators(locations.at(i).toString());
			listWidgetMusicLocations->addItem(new QListWidgetItem(convertedPath));
		}
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
		QStandardItem *languageItem = new QStandardItem();
		languageItem->setData(i, Qt::DisplayRole);
		languageItem->setData(i, Qt::UserRole+1);
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
	connect(seekTimeSpinBox, SIGNAL(valueChanged(int)), settings, SLOT(setPlaybackSeekTime(int)));
	connect(radioButtonKeepPlaylists, &QRadioButton::toggled, settings, &Settings::setPlaybackKeepPlaylists);

	seekTimeSpinBox->setValue(settings->playbackSeekTime()/1000);
	if (settings->playbackKeepPlaylists()) {
		radioButtonKeepPlaylists->setChecked(true);
	} else {
		radioButtonClearPlaylists->setChecked(true);
	}

	if (settings->copyTracksFromPlaylist()) {
		radioButtonDDCopyPlaylistTracks->setChecked(true);
	} else {
		radioButtonDDMovePlaylistTracks->setChecked(true);
	}

	// Fifth panel: drag and drop
	connect(radioButtonDDOpenPopup, &QRadioButton::toggled, settings, &Settings::setDragAndDropBehaviour);
	connect(radioButtonDDAddToLibrary, &QRadioButton::toggled, settings, &Settings::setDragAndDropBehaviour);
	connect(radioButtonDDAddToPlaylist, &QRadioButton::toggled, settings, &Settings::setDragAndDropBehaviour);
	connect(radioButtonDDCopyPlaylistTracks, &QRadioButton::toggled, settings, &Settings::setCopyTracksFromPlaylist);

	QRadioButton *radioButtonDD = this->findChild<QRadioButton*>(settings->dragAndDropBehaviour());
	if (radioButtonDD) {
		radioButtonDD->setChecked(true);
	}

	QTranslator qtTranslator;
	bool b = qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
	qDebug() << "qt translation loaded?" << b << QLocale::system().name() << QLibraryInfo::location(QLibraryInfo::TranslationsPath);
	b = QApplication::installTranslator(&qtTranslator);
	qDebug() << "translator installed?" << b;

	// Load the language of the application
	QString lang = languages.value(Settings::getInstance()->language());
	t.load(lang);
	QApplication::installTranslator(&t);
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
void CustomizeOptionsDialog::closeEvent(QCloseEvent * /* event */)
{
    QString libraryPath = QStandardPaths::writableLocation(QStandardPaths::DataLocation).append(QDir::separator()).append("library.mmmmp");
    if (Settings::getInstance()->musicLocations().isEmpty() && QFile::exists(libraryPath)) {
        QFile::remove(libraryPath);
	}
	if (musicLocationsChanged) {
		emit musicLocationsHaveChanged(true);
		musicLocationsChanged = false;
	}
}

void CustomizeOptionsDialog::addMusicLocation(const QString &musicLocation)
{
	bool existingItem = false;
	for (int i=0; i<listWidgetMusicLocations->count(); i++) {
		QListWidgetItem *item = listWidgetMusicLocations->item(i);
		if (item->text() == QDir::toNativeSeparators(musicLocation)) {
			existingItem = true;
			break;
		}
	}

	if (!existingItem) {
		listWidgetMusicLocations->addItem(new QListWidgetItem(QDir::toNativeSeparators(musicLocation)));
		pushButtonDeleteLocation->setEnabled(true);

		Settings *settings = Settings::getInstance();
		if (settings->musicLocations().isEmpty()) {
			delete listWidgetMusicLocations->takeItem(0);
		}
		settings->addMusicLocation(QDir::fromNativeSeparators(musicLocation));

		// Scan the hard-drive once again
		musicLocationsChanged = true;
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

/** Change language at runtime. */
void CustomizeOptionsDialog::changeLanguage(QModelIndex index)
{
	QString lang = languages.value(index.data().toString());
	Settings *settings = Settings::getInstance();

	// If the language is successfully loaded, tells every widget that they need to be redisplayed
	if (!lang.isEmpty() && lang != settings->language() && t.load(lang)) {
		settings->setLanguage(index.data().toString());
		QApplication::installTranslator(&t);
	} else {
		labelStatusLanguage->setText(tr("No translation is available for this language :("));
	}
}

/** Redefined to initialize theme from settings. */
void CustomizeOptionsDialog::open()
{
	Settings *settings = Settings::getInstance();
	foreach(MediaButton *b, parent()->findChildren<MediaButton*>()) {
		QPushButton *button = findChild<QPushButton*>(b->objectName());
		if (button) {
			button->setIcon(b->icon());
			button->setEnabled(settings->isVisible(b));
			button->setChecked(b->isChecked());
		}
	}
	retranslateUi(this);
	QDialog::open();
}

void CustomizeOptionsDialog::setExternalDragDropPreference(QToolButton *toolButton)
{
	if (toolButton->objectName().toLower().contains("library")) {
		radioButtonDDAddToLibrary->toggle();
	} else {
		radioButtonDDAddToPlaylist->toggle();
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
		QListWidgetItem *current = listWidgetMusicLocations->takeItem(row);

		settings->removeMusicLocation(QDir::fromNativeSeparators(current->text()));
		delete current;

		// Scan the hard-drive once again
		musicLocationsChanged = true;

		if (listWidgetMusicLocations->count() == 0) {
			pushButtonDeleteLocation->setEnabled(false);
			listWidgetMusicLocations->addItem(new QListWidgetItem(tr("Add some music locations here")));
		}
	}
}
