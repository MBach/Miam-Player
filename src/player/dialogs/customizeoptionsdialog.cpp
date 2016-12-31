#include "customizeoptionsdialog.h"

#include <model/sqldatabase.h>
#include <flowlayout.h>
#include <musicsearchengine.h>
#include <settings.h>
#include <settingsprivate.h>
#include "pluginmanager.h"

#include <QDesktopWidget>
#include <QDir>
#include <QFileDialog>
#include <QFileIconProvider>
#include <QLibraryInfo>
#include <QSqlQuery>
#include <QStandardPaths>

#include <QtDebug>

#include <scrollbar.h>

CustomizeOptionsDialog::CustomizeOptionsDialog(PluginManager *pluginManager, QWidget *)
	: QDialog(nullptr)
	, _pluginManager(pluginManager)
{
	setupUi(this);
	listWidget->verticalScrollBar()->deleteLater();
	listWidget->setVerticalScrollBar(new ScrollBar(Qt::Vertical, this));
	listWidget->setAttribute(Qt::WA_MacShowFocusRect, false);
	listWidgetMusicLocations->setAttribute(Qt::WA_MacShowFocusRect, false);

	this->setAttribute(Qt::WA_DeleteOnClose);

	SettingsPrivate *settings = SettingsPrivate::instance();

	// First panel: library
	connect(radioButtonSearchAndExclude, &QRadioButton::toggled, settings, &SettingsPrivate::setSearchAndExcludeLibrary);
	connect(pushButtonAddLocation, &QPushButton::clicked, this, &CustomizeOptionsDialog::openLibraryDialog);
	connect(pushButtonDeleteLocation, &QPushButton::clicked, this, &CustomizeOptionsDialog::deleteMusicLocation);

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
			if (path.isEmpty()) {
				continue;
			}
			QIcon icon = QFileIconProvider().icon(QFileInfo(path));
			listWidgetMusicLocations->addItem(new QListWidgetItem(icon, QDir::toNativeSeparators(path), listWidgetMusicLocations));
			if (musicLocations.isEmpty() || musicLocations.first() != path) {
				comboBoxDefaultFileExplorer->addItem(icon, QDir::toNativeSeparators(path));
			}
		}
		if (listWidgetMusicLocations->count() > 0) {
			listWidgetMusicLocations->setCurrentRow(0);
			pushButtonDeleteLocation->setEnabled(true);
		}
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
	connect(radioButtonEnableMonitorFS, &QRadioButton::toggled, settings, &SettingsPrivate::setMonitorFileSystem);

	// Second panel: languages
	FlowLayout *flowLayout = new FlowLayout(widgetLanguages, 30, 75, 75);
	widgetLanguages->setLayout(flowLayout);

	QDir dir(":/languages");
	for (QString i : dir.entryList()) {

		// Add a new country flag
		QToolButton *languageButton = new QToolButton(this);
		languageButton->setIconSize(QSize(48, 32));
		languageButton->setIcon(QIcon(dir.filePath(i)));
		languageButton->setText(i);
		languageButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		languageButton->setAutoRaise(true);
		flowLayout->addWidget(languageButton);
		connect(languageButton, &QToolButton::clicked, this, &CustomizeOptionsDialog::changeLanguage);

		// Pick the right button in User Interface
		if (i == settings->language()) {
			languageButton->setDown(true);
		}
	}

	// Third panel: @see initShortcuts
	this->initShortcuts();

	// Fourth panel: playback
	seekTimeSpinBox->setValue(settings->playbackSeekTime()/1000);
	connect(seekTimeSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), settings, &SettingsPrivate::setPlaybackSeekTime);

	switch (settings->playbackDefaultActionForClose()) {
	case SettingsPrivate::PDA_AskUserForAction:
		radioButtonAskAction->setChecked(true);
		break;
	case SettingsPrivate::PDA_SaveOnClose:
		radioButtonSavePlaylist->setChecked(true);
		break;
	case SettingsPrivate::PDA_DiscardOnClose:
		radioButtonDiscardPlaylist->setChecked(true);
		break;
	}
	connect(radioButtonAskAction, &QRadioButton::toggled, this, [=]() { settings->setPlaybackCloseAction(SettingsPrivate::PDA_AskUserForAction); });
	connect(radioButtonSavePlaylist, &QRadioButton::toggled, this, [=]() { settings->setPlaybackCloseAction(SettingsPrivate::PDA_SaveOnClose); });
	connect(radioButtonDiscardPlaylist, &QRadioButton::toggled, this, [=]() { settings->setPlaybackCloseAction(SettingsPrivate::PDA_DiscardOnClose); });

	settings->playbackKeepPlaylists() ? radioButtonKeepPlaylists->setChecked(true) : radioButtonClearPlaylists->setChecked(true);
	connect(radioButtonKeepPlaylists, &QRadioButton::toggled, settings, &SettingsPrivate::setPlaybackKeepPlaylists);

	settings->playbackRestorePlaylistsAtStartup() ? radioButtonRestorePlaylists->setChecked(true) : radioButtonDontRestorePlaylists->setChecked(true);
	connect(radioButtonRestorePlaylists, &QRadioButton::toggled, settings, &SettingsPrivate::setPlaybackRestorePlaylistsAtStartup);

	// Fifth panel: drag and drop
	switch (settings->dragDropAction()) {
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

	settings->copyTracksFromPlaylist() ? radioButtonDDCopyPlaylistTracks->setChecked(true) : radioButtonDDMovePlaylistTracks->setChecked(true);

	connect(radioButtonDDOpenPopup, &QRadioButton::toggled, this, [=]() { settings->setDragDropAction(SettingsPrivate::DD_OpenPopup); });
	connect(radioButtonDDAddToLibrary, &QRadioButton::toggled, this, [=]() { settings->setDragDropAction(SettingsPrivate::DD_AddToLibrary); });
	connect(radioButtonDDAddToPlaylist, &QRadioButton::toggled, this, [=]() { settings->setDragDropAction(SettingsPrivate::DD_AddToPlaylist); });
	connect(radioButtonDDCopyPlaylistTracks, &QRadioButton::toggled, settings, &SettingsPrivate::setCopyTracksFromPlaylist);

	// Sixth panel: advanced
	bool isRemote = settings->isRemoteControlEnabled();
	enableRemoteControlCheckBox->setChecked(isRemote);
	remoteControlPortSpinBox->setValue(settings->remoteControlPort());

	remoteControlLabelPort->setEnabled(isRemote);
	remoteControlPortSpinBox->setEnabled(isRemote);

	connect(remoteControlPortSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), settings, &SettingsPrivate::setRemoteControlPort);
	connect(enableRemoteControlCheckBox, &QCheckBox::toggled, this, &CustomizeOptionsDialog::toggleRemoteControl);

	// Seventh panel: plugins
	this->initPlugins();
	connect(pluginSummaryTableWidget, &QTableWidget::itemChanged, this, &CustomizeOptionsDialog::togglePlugin);

	// Restore geometry
	this->restoreGeometry(settings->value("customizeOptionsDialogGeometry").toByteArray());
	listWidget->setCurrentRow(settings->value("customizeOptionsDialogCurrentTab", 0).toInt());
}

/** Redefined to be able to retransltate User Interface at runtime. */
void CustomizeOptionsDialog::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange) {
		this->retranslateUi(this);
	} else {
		QDialog::changeEvent(event);
	}
}

/** Redefined to add custom behaviour. */
void CustomizeOptionsDialog::closeEvent(QCloseEvent *e)
{
	SettingsPrivate *settings = SettingsPrivate::instance();
	settings->setValue("customizeOptionsDialogGeometry", saveGeometry());
	settings->setValue("customizeOptionsDialogCurrentTab", listWidget->currentRow());

	// Drive is scanned only when the popup is closed
	this->updateMusicLocations();

	QDialog::closeEvent(e);
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

void CustomizeOptionsDialog::initPlugins()
{
	QStringList failedOnes;
	for (PluginInfo plugin : SettingsPrivate::instance()->plugins()) {

		QTableWidgetItem *checkbox = new QTableWidgetItem();
		if (plugin.isEnabled()) {
			checkbox->setCheckState(Qt::Checked);
		} else {
			checkbox->setCheckState(Qt::Unchecked);
		}

		// If plugin brings its own UI, add a new page in the list
		bool pluginHasFailed = false;
		if (plugin.isConfigurable() && plugin.isEnabled()) {
			BasicPlugin *p = _pluginManager->loadedPlugins().value(plugin.absFilePath());
			if (p) {
				tabPlugins->addTab(p->configPage(), p->name());
			} else {
				pluginHasFailed = true;
			}
		}

		// Check first if it's possible to add the plugin into the list
		if (pluginHasFailed) {
			// Plugin might has been loaded elsewhere and there's an artifact in the settings
			failedOnes << plugin.pluginName();
		} else {
			// Add name, state and version info on a summary page
			QVariant v = QVariant::fromValue(plugin);
			checkbox->setData(Qt::EditRole, v);
			int row = pluginSummaryTableWidget->rowCount();
			pluginSummaryTableWidget->insertRow(row);
			pluginSummaryTableWidget->setItem(row, 0, new QTableWidgetItem(plugin.pluginName()));
			pluginSummaryTableWidget->setItem(row, 1, checkbox);
			pluginSummaryTableWidget->setItem(row, 2, new QTableWidgetItem(plugin.version()));
		}
	}
	if (!failedOnes.isEmpty()) {
		_pluginManager->alertUser(failedOnes);
	}
}

/** Third panel in this dialog: shorcuts has to be initialized in the end. */
void CustomizeOptionsDialog::initShortcuts()
{
	Settings *settings = Settings::instance();
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

/** Adds a external music locations in the library (Drag & Drop). */
void CustomizeOptionsDialog::addMusicLocations(const QList<QDir> &dirs, bool immediateRescan)
{
	for (QDir musicLocation : dirs) {
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

	this->updateMusicLocations(immediateRescan);
}

/** Application can be retranslated dynamically at runtime. */
void CustomizeOptionsDialog::changeLanguage()
{
	QToolButton *languageButton = qobject_cast<QToolButton*>(sender());
	if (!languageButton) {
		return;
	}

	auto settings = SettingsPrivate::instance();
	bool b = settings->setLanguage(languageButton->text());
	if (b) {
		labelStatusLanguage->setText(QApplication::translate("CustomizeOptionsDialog", "Translation status: OK!"));
	} else {
		labelStatusLanguage->setText(tr("No translation is available for this language :("));
	}

	for (QToolButton *b : this->findChildren<QToolButton*>()) {
		b->setDown(false);
	}
	languageButton->setDown(true);

	if (b) {
		this->retranslateUi(this);
	}
}

/** Verify that one hasn't tried to bind a key twice. */
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
void CustomizeOptionsDialog::deleteMusicLocation()
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

/** Open a dialog for letting the user to choose a music directory. */
void CustomizeOptionsDialog::openLibraryDialog()
{
	QString libraryPath = QFileDialog::getExistingDirectory(this, tr("Select a location of your music"),
		QStandardPaths::standardLocations(QStandardPaths::MusicLocation).first(), QFileDialog::ShowDirsOnly);
	if (!libraryPath.isEmpty()) {
		this->addMusicLocations({ QDir(libraryPath) }, false);
	}
}

/** Load or unload a plugin when one is switching a checkbox in the options. */
void CustomizeOptionsDialog::togglePlugin(QTableWidgetItem *item)
{
	// Checkboxes are in the second column
	if (item->column() == 1) {
		QVariant vPluginInfo = item->data(Qt::EditRole);
		PluginInfo pluginInfo = vPluginInfo.value<PluginInfo>();
		if (item->checkState() == Qt::Checked) {

			// Try to reload plugin
			if (_pluginManager->loadPlugin(pluginInfo.absFilePath())) {
				BasicPlugin *p = _pluginManager->loadedPlugins().value(pluginInfo.absFilePath());
				if (p) {
					tabPlugins->addTab(p->configPage(), p->name());
				} else {
					_pluginManager->alertUser(QStringList(pluginInfo.absFilePath()));
				}
			} else {
				_pluginManager->alertUser(QStringList(pluginInfo.absFilePath()));
			}

		} else if (_pluginManager->unloadPlugin(pluginInfo.absFilePath())) {
			for (int i = 1; i < tabPlugins->count(); i++) {
				if (tabPlugins->tabText(i) == pluginInfo.pluginName()) {
					tabPlugins->removeTab(i);
					break;
				}
			}
		}
	}
}

void CustomizeOptionsDialog::toggleRemoteControl(bool enabled)
{
	qDebug() << Q_FUNC_INFO;
	remoteControlLabelPort->setEnabled(enabled);
	remoteControlPortSpinBox->setEnabled(enabled);
	SettingsPrivate::instance()->setRemoteControlEnabled(enabled);
}

/** Check if music locations have changed in order to rescan the filesystem. */
void CustomizeOptionsDialog::updateMusicLocations(bool immediateRescan)
{
	SettingsPrivate *settings = SettingsPrivate::instance();
	QStringList savedLocations = settings->musicLocations();
	savedLocations.sort();
	QStringList newLocations;
	for (int i = 0; i < listWidgetMusicLocations->count(); i++) {
		QString newLocation = listWidgetMusicLocations->item(i)->text();
		if (QString::compare(newLocation, tr("Add some music locations here")) != 0) {
			newLocations << newLocation;
		}
	}
	newLocations.sort();

	bool musicLocationsAreIdenticals = true;
	if (savedLocations.size() == newLocations.size()) {
		for (int i = 0; i < newLocations.count(); i++) {
			musicLocationsAreIdenticals = musicLocationsAreIdenticals && (QString::compare(savedLocations.at(i), newLocations.at(i)) == 0);
		}
	} else {
		musicLocationsAreIdenticals = false;
	}

	// Trigger the MainWindow (and then the SearchEngine) to restart a scan
	if (!musicLocationsAreIdenticals) {

		QStringList removedLocations;
		for (QString savedLocation : savedLocations) {
			if (!newLocations.contains(savedLocation)) {
				removedLocations << savedLocation;
			}
		}
		SqlDatabase db;

		// Remove old locations from database cache
		db.transaction();
		for (QString removedLocation : removedLocations) {
			QSqlQuery syncDb(db);
			syncDb.setForwardOnly(true);
			syncDb.prepare("DELETE FROM tracks WHERE uri LIKE :path ");
			syncDb.bindValue(":path", QDir::fromNativeSeparators(removedLocation) + "%");
			syncDb.exec();
			syncDb.exec("DELETE FROM albums WHERE id NOT IN (SELECT DISTINCT albumId FROM tracks)");
			syncDb.exec("DELETE FROM artists WHERE id NOT IN (SELECT DISTINCT artistId FROM tracks)");
		}
		db.commit();

		if (immediateRescan) {
			settings->setMusicLocations(newLocations);
			settings->sync();
		}
	}
}
