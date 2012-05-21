#include "customizethemedialog.h"

#include <QDesktopServices>
#include <QFileDialog>

CustomizeThemeDialog::CustomizeThemeDialog(QWidget *parent) :
	QDialog(parent), targetedColor(NULL)
{
	setupUi(this);

	mainWindow = qobject_cast<MainWindow *>(parent);
	colorDialog = new QColorDialog(this);
	colorDialog->setOptions(QColorDialog::ShowAlphaChannel);
	buttonsListBox->setVisible(false);

	foreach(MediaButton *b, mainWindow->mediaButtons) {
		connect(themeComboBox, SIGNAL(currentIndexChanged(QString)), b, SLOT(setIconFromTheme(QString)));
		connect(sizeButtonsSpinBox, SIGNAL(valueChanged(int)), b, SLOT(setSize(int)));
	}

	// Select button theme and size
	Settings *settings = Settings::getInstance();
	connect(themeComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(setThemeNameAndDialogButtons(QString)));
	connect(sizeButtonsSpinBox, SIGNAL(valueChanged(int)), settings, SLOT(setButtonsSize(int)));

	// Hide buttons or not
	foreach(MediaButton *b, mainWindow->mediaButtons) {
		QCheckBox *checkBox = findChild<QCheckBox *>(b->objectName().replace("Button", "CheckBox"));
		if (checkBox) {
			connect(checkBox, SIGNAL(toggled(bool)), b, SLOT(setVisible(bool)));
			connect(b, SIGNAL(visibilityChanged(MediaButton*, bool)), settings, SLOT(setVisible(MediaButton*, bool)));
		}

		// Connect a file dialog to every button if one wants to customize everything
		QPushButton *pushButton = buttonsListBox->findChild<QPushButton *>(b->objectName().remove("Button"));
		connect(pushButton, SIGNAL(clicked()), this, SLOT(openChooseIconDialog()));

		connect(flatButtonsCheckBox, SIGNAL(toggled(bool)), b, SLOT(makeFlat(bool)));
	}
	connect(flatButtonsCheckBox, SIGNAL(toggled(bool)), settings, SLOT(setButtonsFlat(bool)));


	// Fonts
	connect(fontComboBoxPlaylist, SIGNAL(currentFontChanged(QFont)), this, SLOT(updateFontFamily(QFont)));
	connect(fontComboBoxLibrary, SIGNAL(currentFontChanged(QFont)), this, SLOT(updateFontFamily(QFont)));
	connect(fontComboBoxMenus, SIGNAL(currentFontChanged(QFont)), this, SLOT(updateFontFamily(QFont)));
	connect(spinBoxPlaylist, SIGNAL(valueChanged(int)), this, SLOT(updateFontSize(int)));
	connect(spinBoxLibrary, SIGNAL(valueChanged(int)), this, SLOT(updateFontSize(int)));
	connect(spinBoxMenus, SIGNAL(valueChanged(int)), this, SLOT(updateFontSize(int)));

	// Colors
	connect(enableAlternateBGRadioButton, SIGNAL(toggled(bool)), settings, SLOT(setColorsAlternateBG(bool)));
	connect(enableAlternateBGRadioButton, SIGNAL(toggled(bool)), this, SLOT(changeColor()));
	foreach (QToolButton *b, tabWidgetColors->findChildren<QToolButton*>()) {
		connect(b, SIGNAL(clicked()), this, SLOT(showColorDialog()));
		connect(colorDialog, SIGNAL(accepted()), this, SLOT(changeColor()));
	}

	// Library
	connect(checkBoxAlphabeticalSeparators, SIGNAL(toggled(bool)), this, SLOT(displayAlphabeticalSeparators(bool)));
	connect(checkBoxDisplayCovers, SIGNAL(toggled(bool)), this, SLOT(displayCovers(bool)));
	connect(spinBoxCoverSize, SIGNAL(valueChanged(int)), mainWindow->library, SIGNAL(sizeOfCoversChanged(int)));

	this->loadTheme();
}

void CustomizeThemeDialog::showColorDialog()
{
	targetedColor = tabWidgetColors->findChild<QWidget*>(sender()->objectName().replace("ToolButton", "Widget"));
	if (targetedColor) {
		colorDialog->show();
		this->hide();
	}
}

// Make some sublclasses in a strategy pattern and execute something like this
// targetedColor->updateAssociatedElements();
// Below is just the first proof-of-concept working code
void CustomizeThemeDialog::changeColor()
{
	if (targetedColor) {
		QColor selectedColor = colorDialog->currentColor();
		targetedColor->setStyleSheet("QWidget{ border: 1px solid black; background-color: " + selectedColor.name() + ";} ");
		// Playlists
		if (targetedColor == bgPrimaryColorWidget) {
			selectedColor = selectedColor.toHsv();
			QColor alternateColor;
			if (selectedColor.value() > 9) {
				alternateColor.setHsv(selectedColor.hue(), selectedColor.saturation(), selectedColor.value() - 9);
			} else {
				alternateColor.setHsv(selectedColor.hue(), selectedColor.saturation(), selectedColor.value() + 9);
			}
			QList<Playlist*> playlists = mainWindow->findChildren<Playlist*>();
			QString styleSheet;

			foreach(Playlist *playlist, playlists) {
				bool b = Settings::getInstance()->colorsAlternateBG();
				if (b) {
					styleSheet = "background-color: " + selectedColor.name() + "; alternate-background-color: " + alternateColor.name() + ';';
				} else {
					styleSheet = "background-color: " + selectedColor.name() + ';';
				}
				playlist->setAlternatingRowColors(b);
				playlist->setStyleSheet(styleSheet);
				styleSheet = "::section { background-color: " + selectedColor.name() + "; }";
				playlist->horizontalHeader()->setStyleSheet(styleSheet);
			}
		} else if (targetedColor == itemColorWidget) {
			qDebug() << "todo";
		}
		this->show();
	}
}

/** Load theme at startup. */
void CustomizeThemeDialog::loadTheme()
{
	Settings *settings = Settings::getInstance();
	sizeButtonsSpinBox->setValue(settings->buttonsSize());
	flatButtonsCheckBox->setChecked(settings->buttonsFlat());

	// Select the right drop-down item according to the theme
	int i=0;
	while (settings->theme() != themeComboBox->itemText(i).toLower()) {
		i++;
	}
	themeComboBox->setCurrentIndex(i);

	// Buttons
	foreach(MediaButton *b, mainWindow->mediaButtons) {
		// Display or hide buttons in the main window interface
		bool state = settings->isVisible(b);
		b->setVisible(state);

		// Check or uncheck checkboxes in this customize interface
		QCheckBox *checkBox = findChild<QCheckBox *>(b->objectName().replace("Button", "CheckBox"));
		if (checkBox) {
			checkBox->setChecked(state);
		}

		// Display customs icons, if any
		QPushButton *pushButton = findChild<QPushButton *>(b->objectName().remove("Button"));
		pushButton->setIcon(b->icon());
	}
	mainWindow->repeatButton->setChecked(settings->repeatPlayBack());
	mainWindow->shuffleButton->setChecked(settings->shufflePlayBack());

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

/** Redefined to initialize favorites from settings. */
void CustomizeThemeDialog::open()
{
	// Change the label that talks about star delegates
	bool starDelegateState = Settings::getInstance()->isStarDelegates();
	labelLibraryDelegates->setEnabled(starDelegateState);
	radioButtonShowNeverScoredTracks->setEnabled(starDelegateState);
	radioButtonHideNeverScoredTracks->setEnabled(starDelegateState);
	if (starDelegateState) {
		labelLibraryDelegatesState->setText(tr("Favorites are currently enabled"));
	} else {
		labelLibraryDelegatesState->setText(tr("Favorites are currently disabled"));
	}
	QDialog::open();
}

void CustomizeThemeDialog::openChooseIconDialog()
{
	QPushButton *button = qobject_cast<QPushButton *>(sender());
	MediaButton *b = mainWindow->findChild<MediaButton*>(button->objectName()+"Button");
	QString path = QFileDialog::getOpenFileName(this, tr("Choose your custom icon"), QDesktopServices::storageLocation(QDesktopServices::PicturesLocation), tr("Pictures (*.jpg *.jpeg *.png)"));

	Settings *settings = Settings::getInstance();
	settings->setCustomIcon(b, path);
	// Reset the custom icon
	if (path.isEmpty()) {
		path = ":/player/" + settings->theme() + "/" + button->objectName();
	}
	button->setIcon(QIcon(path));
	b->setIcon(QIcon(path));
}

/** Changes the current theme and updates this dialog too. */
void CustomizeThemeDialog::setThemeNameAndDialogButtons(QString newTheme) {
	// Updates dynamically this Dialog
	Settings *settings = Settings::getInstance();
	// Check for each button if there is a custom icon
	foreach(QPushButton *button, buttonsListBox->findChildren<QPushButton*>()) {
		MediaButton *mediaButton = mainWindow->findChild<MediaButton*>(button->objectName()+"Button");
		// Keep the custom icon provided by one
		if (settings->hasCustomIcon(mediaButton)) {
			button->setIcon(QIcon(settings->customIcon(mediaButton)));
		} else {
			button->setIcon(QIcon(":/player/" + newTheme.toLower() + "/" + button->objectName()));
		}
	}
	settings->setThemeName(newTheme);
}

/** Displays covers or not in the library. */
void CustomizeThemeDialog::displayCovers(bool b)
{
	Settings::getInstance()->setCovers(b);
	mainWindow->library->beginPopulateTree();
}

/** Displays alphabecical separators or not in the library. */
void CustomizeThemeDialog::displayAlphabeticalSeparators(bool b)
{
	Settings::getInstance()->setToggleSeparators(b);
	mainWindow->library->beginPopulateTree();
}

/** Updates the font family of a specific component. */
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

/** Updates the font size of a specific component. */
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
