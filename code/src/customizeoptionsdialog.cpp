#include "customizeoptionsdialog.h"

#include "mainwindow.h"
#include "settings.h"

#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>

#include <QtDebug>

CustomizeOptionsDialog::CustomizeOptionsDialog(QWidget *parent) :
	QDialog(parent), musicLocationsChanged(false)
{
	setupUi(this);

	QStandardItemModel *languageModel = new QStandardItemModel(this);
	listViewLanguages->setModel(languageModel);
	listViewLanguages->setViewMode(QListView::IconMode);
	listViewLanguages->setDragDropMode(QAbstractItemView::NoDragDrop);
	listViewLanguages->setGridSize(QSize(168, 64));

	QDir dir(":/languages");
	QStringList fileNames = dir.entryList();
	Settings *settings = Settings::getInstance();
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

	// First panel: library
	connect(radioButtonActivateDelegates, SIGNAL(toggled(bool)), this, SLOT(setDelegates(bool)));
	connect(pushButtonAddLocation, SIGNAL(clicked()), this, SLOT(openLibraryDialog()));
	connect(pushButtonDeleteLocation, SIGNAL(clicked()), this, SLOT(deleteSelectedLocation()));

	// Second panel: language
	connect(listViewLanguages, SIGNAL(clicked(QModelIndex)), this, SLOT(changeLanguage(QModelIndex)));

	// Third panel: shorcuts
	/// todo
}

/** Load the language saved in settings when the app is loading. Called only once per launch. */
void CustomizeOptionsDialog::loadLanguage()
{
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
			"CustomizeOptionsDialog", "Add some music locations here", 0, QApplication::UnicodeUTF8));
	}
	Ui::CustomizeOptionsDialog::retranslateUi(dialog);
}

/** Redefined to add custom behaviour. */
void CustomizeOptionsDialog::closeEvent(QCloseEvent * /* event */)
{
	if (Settings::getInstance()->musicLocations().isEmpty() && QFile::exists("library.mmmmp")) {
		QFile::remove("library.mmmmp");
	}
	if (musicLocationsChanged) {
		emit musicLocationsHasChanged(true);
		musicLocationsChanged = false;
	}
}

void CustomizeOptionsDialog::setDelegates(bool value)
{
	Settings *settings = Settings::getInstance();
	settings->setDelegates(value);
	//MainWindow *mainWindow = qobject_cast<MainWindow*>(parent);
	//emit mainWindow->delegateStateChanged();
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

/** Open a dialog for letting the user to choose a music directory. */
void CustomizeOptionsDialog::openLibraryDialog()
{
	QString libraryPath = QFileDialog::getExistingDirectory(this, tr("Select a location of your music"),
		QDesktopServices::storageLocation(QDesktopServices::MusicLocation), QFileDialog::ShowDirsOnly);
	if (!libraryPath.isEmpty()) {
		bool existingItem = false;

		for (int i=0; i<listWidgetMusicLocations->count(); i++) {
			QListWidgetItem *item = listWidgetMusicLocations->item(i);
			if (item->text() == libraryPath) {
				existingItem = true;
				break;
			}
		}

		if (!existingItem) {
			listWidgetMusicLocations->addItem(new QListWidgetItem(libraryPath));
			pushButtonDeleteLocation->setEnabled(true);

			Settings *settings = Settings::getInstance();
			if (settings->musicLocations().isEmpty()) {
				delete listWidgetMusicLocations->takeItem(0);
			}
			settings->addMusicLocation(QDir::fromNativeSeparators(libraryPath));

			// Scan the hard-drive once again
			musicLocationsChanged = true;
		}
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
