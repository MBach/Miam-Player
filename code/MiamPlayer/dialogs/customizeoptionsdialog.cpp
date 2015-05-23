#include "customizeoptionsdialog.h"

#include "flowlayout.h"
#include "mainwindow.h"
#include "pluginmanager.h"
#include "settingsprivate.h"
#include "musicsearchengine.h"

#include <QDesktopWidget>
#include <QDir>
#include <QFileDialog>
#include <QFileIconProvider>
#include <QLibraryInfo>
#include <QStandardPaths>

#include <QtDebug>

CustomizeOptionsDialog::CustomizeOptionsDialog(QWidget *parent) :
	QDialog(parent)
{
	setupUi(this);
	listWidget->setAttribute(Qt::WA_MacShowFocusRect, false);
	listWidgetMusicLocations->setAttribute(Qt::WA_MacShowFocusRect, false);

	this->setWindowFlags(Qt::Tool);
	this->setModal(true);

	SettingsPrivate *settings = SettingsPrivate::instance();

	// First panel: library
	connect(radioButtonSearchAndExclude, &QRadioButton::toggled, settings, &SettingsPrivate::setSearchAndExcludeLibrary);
	connect(pushButtonAddLocation, &QPushButton::clicked, this, &CustomizeOptionsDialog::openLibraryDialog);
	connect(pushButtonDeleteLocation, &QPushButton::clicked, this, &CustomizeOptionsDialog::deleteSelectedLocation);

	QStringList musicLocations = QStandardPaths::standardLocations(QStandardPaths::MusicLocation);
	if (!musicLocations.isEmpty()) {
		QIcon icon = QFileIconProvider().icon(QFileInfo(musicLocations.first()));
		comboBoxDefaultFileExplorer->addItem(icon, QDir::toNativeSeparators(musicLocations.first()));
	}

	if (settings->librarySearchMode() == SettingsPrivate::LSM_Filter) {
		radioButtonSearchAndExclude->setChecked(true);
	} else {
		radioButtonSearchAndKeep->setChecked(true);
	}

	QStringList locations = settings->musicLocations();
	if (locations.isEmpty()) {
		listWidgetMusicLocations->addItem(new QListWidgetItem(tr("Add some music locations here"), listWidgetMusicLocations));
	} else {
		for (QString path : locations) {
			QIcon icon = QFileIconProvider().icon(QFileInfo(path));
			listWidgetMusicLocations->addItem(new QListWidgetItem(icon, QDir::toNativeSeparators(path), listWidgetMusicLocations));
			if (musicLocations.isEmpty() || musicLocations.first() != path) {
				comboBoxDefaultFileExplorer->addItem(icon, QDir::toNativeSeparators(path));
			}
		}
		listWidgetMusicLocations->setCurrentRow(0);
		pushButtonDeleteLocation->setEnabled(true);
	}


	// Restore default location for the file explorer
	for (int i = 0; i < comboBoxDefaultFileExplorer->count(); i++) {
		if (comboBoxDefaultFileExplorer->itemText(i) == QDir::toNativeSeparators(settings->defaultLocationFileExplorer())) {
			comboBoxDefaultFileExplorer->setCurrentIndex(i);
			break;
		}
	}
	connect(comboBoxDefaultFileExplorer, &QComboBox::currentTextChanged, [=](const QString &location) {
		settings->setDefaultLocationFileExplorer(location);
		emit defaultLocationFileExplorerHasChanged(QDir(location));
	});

	settings->isFileSystemMonitored() ? radioButtonEnableMonitorFS->setChecked(true) : radioButtonDisableMonitorFS->setChecked(true);
	connect(radioButtonEnableMonitorFS, &QRadioButton::toggled, this, [=](bool b) {
		settings->setMonitorFileSystem(b);
		SqlDatabase::instance()->musicSearchEngine()->setWatchForChanges(b);
	});

	// Second panel: languages
	FlowLayout *flowLayout = new FlowLayout(widgetLanguages, 30, 75, 75);
	widgetLanguages->setLayout(flowLayout);

	QDir dir(":/languages");
	for (QString i : dir.entryList()) {

		// If the language is available, then store it
		QFileInfo lang(":/translations/" + i);
		if (lang.exists()) {
			languages.insert(i, lang.filePath());
		}

		// Add a new country flag
		QToolButton *languageButton = new QToolButton(this);
		languageButton->setIconSize(QSize(48, 32));
		languageButton->setIcon(QIcon(dir.filePath(i)));
		languageButton->setText(i);
		languageButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		languageButton->setAutoRaise(true);
		flowLayout->addWidget(languageButton);
		connect(languageButton, &QToolButton::clicked, this, [=]() {
			this->changeLanguage(languageButton->text());
			for (QToolButton *b : this->findChildren<QToolButton*>()) {
				b->setDown(false);
			}
			languageButton->setDown(true);
		});

		// Pick the right button in User Interface
		if (i == settings->language()) {
			languageButton->setDown(true);
		}
	}

	// Third panel: @see initShortcuts

	// Fourth panel: playback
	seekTimeSpinBox->setValue(settings->playbackSeekTime()/1000);
	connect(seekTimeSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), settings, &SettingsPrivate::setPlaybackSeekTime);

	this->initCloseActionForPlaylists();
	connect(radioButtonAskAction, &QRadioButton::toggled, this, [=]() { settings->setPlaybackCloseAction(SettingsPrivate::PL_AskUserForAction); });
	connect(radioButtonSavePlaylist, &QRadioButton::toggled, this, [=]() { settings->setPlaybackCloseAction(SettingsPrivate::PL_SaveOnClose); });
	connect(radioButtonDiscardPlaylist, &QRadioButton::toggled, this, [=]() { settings->setPlaybackCloseAction(SettingsPrivate::PL_DiscardOnClose); });

	settings->playbackKeepPlaylists() ? radioButtonKeepPlaylists->setChecked(true) : radioButtonClearPlaylists->setChecked(true);
	connect(radioButtonKeepPlaylists, &QRadioButton::toggled, settings, &SettingsPrivate::setPlaybackKeepPlaylists);

	settings->playbackRestorePlaylistsAtStartup() ? radioButtonRestorePlaylists->setChecked(true) : radioButtonDontRestorePlaylists->setChecked(true);
	connect(radioButtonRestorePlaylists, &QRadioButton::toggled, settings, &SettingsPrivate::setPlaybackRestorePlaylistsAtStartup);

	// Fifth panel: drag and drop
	this->initDragDropAction();

	settings->copyTracksFromPlaylist() ? radioButtonDDCopyPlaylistTracks->setChecked(true) : radioButtonDDMovePlaylistTracks->setChecked(true);

	connect(radioButtonDDOpenPopup, &QRadioButton::toggled, this, [=]() { settings->setDragDropAction(SettingsPrivate::DD_OpenPopup); });
	connect(radioButtonDDAddToLibrary, &QRadioButton::toggled, this, [=]() { settings->setDragDropAction(SettingsPrivate::DD_AddToLibrary); });
	connect(radioButtonDDAddToPlaylist, &QRadioButton::toggled, this, [=]() { settings->setDragDropAction(SettingsPrivate::DD_AddToPlaylist); });
	connect(radioButtonDDCopyPlaylistTracks, &QRadioButton::toggled, settings, &SettingsPrivate::setCopyTracksFromPlaylist);

	// Load the language of the application
	customTranslator.load(languages.value(SettingsPrivate::instance()->language()));

	// Translate standard buttons (OK, Cancel, ...)
	defaultQtTranslator.load("qt_" + SettingsPrivate::instance()->language(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));

	QApplication::installTranslator(&customTranslator);
	QApplication::installTranslator(&defaultQtTranslator);

	// Restore geometry
	this->restoreGeometry(settings->value("customizeOptionsDialogGeometry").toByteArray());
}

/** Third panel in this dialog: shorcuts has to be initialized in the end. */
void CustomizeOptionsDialog::initShortcuts()
{
	SettingsPrivate *settings = SettingsPrivate::instance();
	QMap<QKeySequenceEdit *, QKeySequence> *defaultShortcuts = new QMap<QKeySequenceEdit *, QKeySequence>();
	QMap<QString, QVariant> shortcutMap = settings->shortcuts();
	for (QKeySequenceEdit *shortcut : shortcutsToolBox->findChildren<QKeySequenceEdit*>()) {
		QKeySequence sequence = shortcut->keySequence();
		defaultShortcuts->insert(shortcut, sequence);

		// Init shortcuts: override default shortcuts with existing ones in settings
		QString shortCutName = shortcut->objectName();
		if (shortcutMap.contains(shortCutName)) {
			sequence = QKeySequence(shortcutMap.value(shortCutName).toString());
			shortcut->setKeySequence(sequence);
		}

		// Load current shortcut
		emit aboutToBindShortcut(shortCutName, sequence);

		// Forward signal to MainWindow
		connect(shortcut, &QKeySequenceEdit::editingFinished, this, &CustomizeOptionsDialog::checkShortcutsIntegrity);

		// Watch all instances of QKeySequenceEdit in case of ambiguously defined shortcut
		shortcut->installEventFilter(this);
	}

	// Clear current shortcut or restore default one
	for (QKeySequenceEdit *shortcut : shortcutsToolBox->findChildren<QKeySequenceEdit*>()) {
		QPushButton *clear = shortcutsToolBox->findChild<QPushButton*>(shortcut->objectName().append("Clear"));
		QPushButton *reset = shortcutsToolBox->findChild<QPushButton*>(shortcut->objectName().append("Reset"));
		connect(clear, &QPushButton::clicked, this, [=]() {
			shortcut->clear();
			emit shortcut->editingFinished();
		});
		connect(reset, &QPushButton::clicked, this, [=]() {
			shortcut->setKeySequence(defaultShortcuts->value(shortcut));
			emit shortcut->editingFinished();
		});
	}
}

/** Is it necessary to redefined this from the UI class just for this init label? */
void CustomizeOptionsDialog::retranslateUi(CustomizeOptionsDialog *dialog)
{
	if (listWidgetMusicLocations->count() > 0 &&
			listWidgetMusicLocations->item(0)->text() == "Add some music locations here") {
		listWidgetMusicLocations->item(0)->setText(QApplication::translate(
			"CustomizeOptionsDialog", "Add some music locations here"));
	}
	Ui::CustomizeOptionsDialog::retranslateUi(dialog);
}

/** Redefined to add custom behaviour. */
void CustomizeOptionsDialog::closeEvent(QCloseEvent *e)
{
	QDialog::closeEvent(e);
	SettingsPrivate *settings = SettingsPrivate::instance();
	settings->setValue("customizeOptionsDialogGeometry", saveGeometry());
	settings->setValue("customizeOptionsDialogCurrentTab", listWidget->currentRow());

	// Drive is scanned only when the popup is closed
	this->updateMusicLocations();
}

/** Redefined to inspect shortcuts. */
bool CustomizeOptionsDialog::eventFilter(QObject *obj, QEvent *e)
{
	QKeySequenceEdit *edit = qobject_cast<QKeySequenceEdit*>(obj);

	// When one is clicking outside the area which is marked as dirty, just remove what has been typed
	if (edit && e->type() == QEvent::FocusOut && edit->property("ambiguous").toBool()) {
		edit->setProperty("ambiguous", false);
		edit->setStyleSheet("");
		edit->clear();
		emit edit->editingFinished();

		// Don't forget to clear ambiguous status for other QKeySequenceEdit widget
		for (QKeySequenceEdit *other : shortcutsToolBox->findChildren<QKeySequenceEdit*>()) {
			if (other->property("ambiguous").toBool()) {
				other->setProperty("ambiguous", false);
				other->setStyleSheet("");
			}
		}
	}
	return QDialog::eventFilter(obj, e);
}

/** Adds a new music location in the library. */
void CustomizeOptionsDialog::addMusicLocation(const QDir &musicLocation)
{
	QString path = musicLocation.absolutePath();
	int existingItem = -1;
	for (int i = 0; i < listWidgetMusicLocations->count(); i++) {
		QListWidgetItem *item = listWidgetMusicLocations->item(i);
		QDir itemDir(item->text());
		if ((QString::compare(itemDir.absolutePath(), path) == 0) ||
			QString::compare(item->text(), tr("Add some music locations here")) == 0) {
			existingItem = i;
			break;
		}
	}
	if (existingItem >= 0) {
		delete listWidgetMusicLocations->takeItem(existingItem);
	}
	QIcon icon = QFileIconProvider().icon(QFileInfo(musicLocation.absolutePath()));
	listWidgetMusicLocations->addItem(new QListWidgetItem(icon, QDir::toNativeSeparators(path), listWidgetMusicLocations));
	pushButtonDeleteLocation->setEnabled(true);

	// Add this music location in the defaults for the file explorer
	QStringList l = QStandardPaths::standardLocations(QStandardPaths::MusicLocation);
	if (l.isEmpty() || l.first() != path) {
		comboBoxDefaultFileExplorer->addItem(icon, QDir::toNativeSeparators(path));
	}
}

/** Adds a external music locations in the library (Drag & Drop). */
void CustomizeOptionsDialog::addMusicLocations(const QList<QDir> &dirs)
{
	for (QDir folder : dirs) {
		this->addMusicLocation(folder);
	}
	this->updateMusicLocations();
}

/** Change language at runtime. */
void CustomizeOptionsDialog::changeLanguage(const QString &language)
{
	QString lang = languages.value(language);
	SettingsPrivate *settings = SettingsPrivate::instance();

	// If the language is successfully loaded, tells every widget that they need to be redisplayed
	if (!lang.isEmpty() && lang != settings->language() && customTranslator.load(lang)) {
		settings->setLanguage(language);
		defaultQtTranslator.load("qt_" + lang, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
		QApplication::installTranslator(&customTranslator);
		/// TODO: reload plugin UI
		QApplication::installTranslator(&defaultQtTranslator);
	} else {
		labelStatusLanguage->setText(tr("No translation is available for this language :("));
	}
}

/** Redefined to initialize theme from settings. */
void CustomizeOptionsDialog::open()
{
	for (MediaButton *b : parent()->findChildren<MediaButton*>()) {
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
	if (SettingsPrivate::instance()->value("customizeOptionsDialogGeometry").isNull()) {
		int w = qApp->desktop()->screenGeometry().width() / 2;
		int h = qApp->desktop()->screenGeometry().height() / 2;
		this->move(w - frameGeometry().width() / 2, h - frameGeometry().height() / 2);
	}
	QDialog::open();
	this->activateWindow();
}

void CustomizeOptionsDialog::checkShortcutsIntegrity()
{
	bool ok = true;

	// Find ambiguous shortcuts
	QMultiMap<int, QKeySequenceEdit*> map;
	for (QKeySequenceEdit *edit : shortcutsToolBox->findChildren<QKeySequenceEdit*>()) {
		// Each sequence can has up to 4 keys
		for (int i = 0; i < edit->keySequence().count(); i++) {
			map.insert(edit->keySequence()[i], edit);
		}
	}
	QMapIterator<int, QKeySequenceEdit*> it(map);
	while (it.hasNext()) {
		it.next();
		QKeySequenceEdit *edit = it.value();

		// Shortcut is defined for more than 1 action -> mark it dirty
		if (map.values(it.key()).size() > 1) {
			edit->setStyleSheet("color: red");
			edit->setProperty("ambiguous", true);
			ok = false;
		}
	}

	if (ok) {
		QKeySequenceEdit *shortcut = qobject_cast<QKeySequenceEdit*>(sender());
		SettingsPrivate::instance()->setShortcut(shortcut->objectName(), shortcut->keySequence());
		emit aboutToBindShortcut(shortcut->objectName(), shortcut->keySequence());
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

	SettingsPrivate *settings = SettingsPrivate::instance();
	QString text;
	if (!settings->musicLocations().isEmpty()) {
		text = listWidgetMusicLocations->item(row)->text();
		delete listWidgetMusicLocations->takeItem(row);

		if (listWidgetMusicLocations->count() == 0) {
			pushButtonDeleteLocation->setEnabled(false);
			listWidgetMusicLocations->addItem(new QListWidgetItem(tr("Add some music locations here"), listWidgetMusicLocations));
		}
	}

	// Udpate default list for the File Explorer
	if (!text.isEmpty()) {
		QStringList musicLocations = QStandardPaths::standardLocations(QStandardPaths::MusicLocation);
		if (!musicLocations.isEmpty() && QDir::toNativeSeparators(musicLocations.first()) != text) {
			int comboRow = comboBoxDefaultFileExplorer->findText(text);
			if (comboRow != -1) {
				comboBoxDefaultFileExplorer->removeItem(comboRow);
			}
		}
	}
}

void CustomizeOptionsDialog::initCloseActionForPlaylists()
{
	switch (SettingsPrivate::instance()->playbackDefaultActionForClose()) {
	case SettingsPrivate::PL_AskUserForAction:
		radioButtonAskAction->setChecked(true);
		break;
	case SettingsPrivate::PL_SaveOnClose:
		radioButtonSavePlaylist->setChecked(true);
		break;
	case SettingsPrivate::PL_DiscardOnClose:
		radioButtonDiscardPlaylist->setChecked(true);
		break;
	}
}

void CustomizeOptionsDialog::initDragDropAction()
{
	switch (SettingsPrivate::instance()->dragDropAction()) {
	case SettingsPrivate::DD_OpenPopup:
		radioButtonDDOpenPopup->setChecked(true);
		break;
	case SettingsPrivate::DD_AddToLibrary:
		radioButtonDDAddToLibrary->setChecked(true);
		break;
	case SettingsPrivate::DD_AddToPlaylist:
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
		this->addMusicLocation(QDir(libraryPath));
	}
}

void CustomizeOptionsDialog::updateMusicLocations()
{
	SettingsPrivate *settings = SettingsPrivate::instance();
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
		emit musicLocationsHaveChanged(savedLocations, newLocations);
	}
}
