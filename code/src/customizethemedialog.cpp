#include "customizethemedialog.h"

CustomizeThemeDialog::CustomizeThemeDialog(QWidget *parent) :
	QDialog(parent)
{
	setupUi(this);

	mainWindow = qobject_cast<MainWindow *>(parent);
	buttonsListBox->setVisible(false);

	MediaButton *b;
	Settings *settings = Settings::getInstance();
	foreach(b, mainWindow->mediaButtons) {
		connect(themeComboBox, SIGNAL(currentIndexChanged(QString)), b, SLOT(setIconFromTheme(QString)));
		connect(sizeButtonsSpinBox, SIGNAL(valueChanged(int)), b, SLOT(setSize(int)));
	}

	// Select button theme and size
	connect(themeComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(setThemeNameAndDialogButtons(QString)));
	connect(sizeButtonsSpinBox, SIGNAL(valueChanged(int)), settings, SLOT(setButtonSize(int)));

	// Hide buttons or not
	foreach(b, mainWindow->mediaButtons) {
		QCheckBox *checkBox = findChild<QCheckBox *>(b->objectName().replace("Button", "CheckBox"));
		connect(checkBox, SIGNAL(toggled(bool)), b, SLOT(setVisible(bool)));
		connect(b, SIGNAL(visibilityChanged(MediaButton*, bool)), settings, SLOT(setVisible(MediaButton*, bool)));
	}

	// Fonts
	connect(fontComboBoxPlaylist, SIGNAL(currentFontChanged(QFont)), this, SLOT(updateFontFamily(QFont)));
	connect(fontComboBoxLibrary, SIGNAL(currentFontChanged(QFont)), this, SLOT(updateFontFamily(QFont)));
	connect(fontComboBoxMenus, SIGNAL(currentFontChanged(QFont)), this, SLOT(updateFontFamily(QFont)));
	connect(spinBoxPlaylist, SIGNAL(valueChanged(int)), this, SLOT(updateFontSize(int)));
	connect(spinBoxLibrary, SIGNAL(valueChanged(int)), this, SLOT(updateFontSize(int)));
	connect(spinBoxMenus, SIGNAL(valueChanged(int)), this, SLOT(updateFontSize(int)));

	// Library
	// Connect a signal to another signal to reach private field in LibraryTreeView class
	connect(checkBoxDisplayCovers, SIGNAL(toggled(bool)), mainWindow->library, SIGNAL(displayCovers(bool)));
	connect(spinBoxCoverSize, SIGNAL(valueChanged(int)), mainWindow->library, SIGNAL(sizeOfCoversChanged(int)));
	// Toggle alphabetical separators in the library
	connect(checkBoxAlphabeticalSeparators, SIGNAL(toggled(bool)), this, SLOT(toggleSeparators(bool)));

	connect(this, SIGNAL(themeChanged()), this, SLOT(loadTheme()));
}

/** Changes the current theme and updates this dialog too. */
void CustomizeThemeDialog::setThemeNameAndDialogButtons(QString newTheme) {
	// Updates dynamically this Dialog
	foreach(QPushButton *button, buttonsListBox->findChildren<QPushButton*>()) {
		button->setIcon(QIcon(":/player/" + newTheme.toLower() + "/" + button->objectName()));
	}
	Settings::getInstance()->setThemeName(newTheme);
}

void CustomizeThemeDialog::toggleSeparators(bool b)
{
	Settings::getInstance()->setToggleSeparators(b);
	emit libraryNeedToBeRepaint();
}

void CustomizeThemeDialog::updateFontFamily(const QFont &font) {
	Settings *settings = Settings::getInstance();
	if (sender()->objectName().contains("Playlist")) {
		settings->setFont(Settings::PLAYLIST, font);
	} else if (sender()->objectName().contains("Library")) {
		settings->setFont(Settings::LIBRARY, font);
	} else if (sender()->objectName().contains("Menus")) {
		settings->setFont(Settings::MENUS, font);
	}
}

void CustomizeThemeDialog::updateFontSize(int i) {
	Settings *settings = Settings::getInstance();
	if (sender()->objectName().contains("Playlist")) {
		settings->setFontPointSize(Settings::PLAYLIST, i);
	} else if (sender()->objectName().contains("Library")) {
		settings->setFontPointSize(Settings::LIBRARY, i);
	} else if (sender()->objectName().contains("Menus")) {
		settings->setFontPointSize(Settings::MENUS, i);
	}
}

void CustomizeThemeDialog::loadTheme()
{
	Settings *settings = Settings::getInstance();
	sizeButtonsSpinBox->setValue(settings->buttonSize());

	// Select the right drop-down item according to the theme
	int i=0;
	while (settings->theme() != themeComboBox->itemText(i).toLower()) {
		i++;
	}
	themeComboBox->setCurrentIndex(i);

	// Buttons
	MediaButton *b;
	foreach(b, mainWindow->mediaButtons) {
		// Display or hide buttons in the main window interface
		bool state = settings->isVisible(b);
		b->setVisible(state);

		// Check or uncheck checkboxes in this customize interface
		QCheckBox *checkBox = findChild<QCheckBox *>(b->objectName().replace("Button", "CheckBox"));
		checkBox->setChecked(state);
	}

	// Change the label that talks about star delegates
	bool starDelegateState = settings->isStarDelegates();
	if (starDelegateState) {
		labelLibraryDelegatesState->setText(tr("Favorites are currently enabled"));
	} else {
		labelLibraryDelegatesState->setText(tr("Favorites are currently disabled"));
	}
	labelLibraryDelegates->setEnabled(starDelegateState);
	radioButtonShowNeverScoredTracks->setEnabled(starDelegateState);
	radioButtonHideNeverScoredTracks->setEnabled(starDelegateState);

	// Fonts
	fontComboBoxPlaylist->setCurrentFont(settings->font(Settings::PLAYLIST));
	fontComboBoxLibrary->setCurrentFont(settings->font(Settings::LIBRARY));
	fontComboBoxMenus->setCurrentFont(settings->font(Settings::MENUS));
	spinBoxPlaylist->setValue(settings->fontSize(Settings::PLAYLIST));
	spinBoxLibrary->setValue(settings->fontSize(Settings::LIBRARY));
	spinBoxMenus->setValue(settings->fontSize(Settings::MENUS));

	// Library
	checkBoxDisplayCovers->setChecked(settings->withCovers());
	spinBoxCoverSize->setValue(settings->coverSize());
	checkBoxAlphabeticalSeparators->setChecked(settings->toggleSeparators());
}
