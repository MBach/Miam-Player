#include "customizethemedialog.h"

#include <QDesktopWidget>
#include <QFileDialog>
#include <QScrollBar>
#include <QStandardPaths>

CustomizeThemeDialog::CustomizeThemeDialog(QWidget *parent) :
	QDialog(parent), targetedColor(NULL)
{
	setupUi(this);

	mainWindow = qobject_cast<MainWindow *>(parent);
	colorDialog = new ColorDialog(this);
	styleSheetUpdater = new StyleSheetUpdater(this);
	buttonsListBox->setVisible(false);

	this->setupActions();
	this->associatePaintableElements();
	this->loadTheme();
}

void CustomizeThemeDialog::associatePaintableElements()
{
	// There are 4 kinds of paintables elements
	bgPrimaryColorWidget->setStyleSheetUpdater(styleSheetUpdater, StyleSheetUpdater::BACKGROUND);
	globalBackgroundColorWidget->setStyleSheetUpdater(styleSheetUpdater, StyleSheetUpdater::GLOBAL_BACKGROUND);
	itemColorWidget->setStyleSheetUpdater(styleSheetUpdater, StyleSheetUpdater::TEXT);
	selectedItemColorWidget->setStyleSheetUpdater(styleSheetUpdater, StyleSheetUpdater::HOVER);

	// Associate instances of Classes to their "preview pane" to dynamically change colors
	/// FIXME: should be class for Playlist because there are some problems when adding/removing playlists during color change
	/// I know this is not really the correct way to use the player, but still, there's a fraking crash!
	QList<QWidget *> bgElements, itemElements;
	for (int i = 0; i < mainWindow->tabPlaylists->count() - 1; i++) {
		Playlist *p = mainWindow->tabPlaylists->playlist(i);
		itemElements << p;
		itemElements << p->horizontalHeader();
		bgElements << p->verticalScrollBar();
	}
	bgElements << mainWindow->library << mainWindow->library->verticalScrollBar() << mainWindow->library->header();
	bgElements << mainWindow->tagEditor->tagEditorWidget << mainWindow->tagEditor->tagEditorWidget->horizontalHeader();
	//bgElements << mainWindow->tagEditor->tagEditorWidget->horizontalScrollBar() << mainWindow->tagEditor->tagEditorWidget->verticalScrollBar();
	itemElements << mainWindow->tabPlaylists << mainWindow->library << mainWindow->widgetSearchBar << mainWindow->leftTabs;
	itemElements << mainWindow->volumeSlider << mainWindow->seekSlider;

	// Background of elements
	bgPrimaryColorWidget->addInstances(bgElements);
	bgPrimaryColorWidget->addInstances(itemElements);

	// General background
	globalBackgroundColorWidget->addInstance(mainWindow);
	globalBackgroundColorWidget->addInstance(mainWindow->splitter);

	// Text
	itemColorWidget->addInstances(itemElements);

	// Background of selected elements
	selectedItemColorWidget->addInstances(bgElements);
	foreach (MediaButton *b, mainWindow->mediaButtons) {
		itemElements << b;
	}
	selectedItemColorWidget->addInstances(itemElements);

	//qDebug() << mainWindow->library->verticalScrollBar()->styleSheet();
}

void CustomizeThemeDialog::setupActions()
{
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
			connect(checkBox, &QCheckBox::toggled, b, &MediaButton::setVisible);
			connect(b, &MediaButton::visibilityChanged, settings, &Settings::setVisible);
		}

		// Connect a file dialog to every button if one wants to customize everything
		QPushButton *pushButton = buttonsListBox->findChild<QPushButton *>(b->objectName().remove("Button"));
		connect(pushButton, &QPushButton::clicked, this, &CustomizeThemeDialog::openChooseIconDialog);
		connect(flatButtonsCheckBox, &QCheckBox::toggled, [=] (bool flat) { b->setFlat(flat); });
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
	connect(enableCustomColorsRadioButton, SIGNAL(toggled(bool)), this, SLOT(toggleCustomColors(bool)));
	connect(enableAlternateBGRadioButton, SIGNAL(toggled(bool)), this, SLOT(toggleAlternativeBackgroundColor(bool)));
	foreach (QToolButton *b, findChildren<QToolButton*>()) {
		connect(b, SIGNAL(clicked()), this, SLOT(showColorDialog()));
	}
	connect(colorDialog, SIGNAL(currentColorChanged(QColor)), this, SLOT(changeColor(QColor)));

	// Library
	connect(checkBoxAlphabeticalSeparators, SIGNAL(toggled(bool)), this, SLOT(displayAlphabeticalSeparators(bool)));
	connect(checkBoxDisplayCovers, SIGNAL(toggled(bool)), this, SLOT(displayCovers(bool)));
	connect(spinBoxCoverSize, SIGNAL(valueChanged(int)), mainWindow->library, SIGNAL(sizeOfCoversChanged(int)));
}

/** Automatically centers the parent window when closing this dialog. */
void CustomizeThemeDialog::closeEvent(QCloseEvent *e)
{
	if (!parentWidget()->isMaximized()) {
		int w = qApp->desktop()->screenGeometry().width() / 2;
		int h = qApp->desktop()->screenGeometry().height() / 2;
		parentWidget()->move(w - parentWidget()->frameGeometry().width() / 2, h - parentWidget()->frameGeometry().height() / 2);
	}
	QDialog::closeEvent(e);
}

/** Shows a color dialog and hides this dialog temporarily.
 * Also, reorder the mainWindow and the color dialog to avoid overlapping, if possible. */
void CustomizeThemeDialog::showColorDialog()
{
	targetedColor = findChild<Reflector*>(sender()->objectName().replace("ToolButton", "Widget"));
	if (targetedColor) {
		// Very important: gets at runtime the elements that will be repaint by one
		colorDialog->setPaintableElements(targetedColor);
		colorDialog->setCurrentColor(targetedColor->color());

		// Moves the color dialog right to the mainWindow
		if (parentWidget()->width() + 20 + colorDialog->width() < qApp->desktop()->availableGeometry().width()) {
			int desktopWidth = qApp->desktop()->availableGeometry().width();
			int w = (desktopWidth - (parentWidget()->width() + 20 + colorDialog->width())) / 2;
			parentWidget()->move(QPoint(w, parentWidget()->pos().y()));
			int h = parentWidget()->y() + parentWidget()->height() / 2 - colorDialog->height() / 2;
			colorDialog->move(parentWidget()->x() + 40 + parentWidget()->width(), h);
		}
		this->hide();
	}
}

void CustomizeThemeDialog::changeColor(QColor selectedColor)
{
	styleSheetUpdater->replace(targetedColor, selectedColor);
}

void CustomizeThemeDialog::toggleAlternativeBackgroundColor(bool b)
{
	Settings::getInstance()->setColorsAlternateBG(b);
	for (int i = 0; i < mainWindow->tabPlaylists->count() - 1; i++) {
		Playlist *p = mainWindow->tabPlaylists->playlist(i);
		p->setAlternatingRowColors(b);
	}
}

void CustomizeThemeDialog::toggleCustomColors(bool b)
{
	Settings::getInstance()->setCustomColors(b);
	for (int i = 0; i < customColorsGridLayout->rowCount(); i++) {
		for (int j = 0; j < customColorsGridLayout->columnCount(); j++) {
			QLayoutItem *item = customColorsGridLayout->itemAtPosition(i, j);
			if (item->widget()) {
				item->widget()->setEnabled(b);
			}
		}
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

	// Colors
	if (settings->colorsAlternateBG()) {
		enableAlternateBGRadioButton->setChecked(true);
	} else {
		disableAlternateBGRadioButton->setChecked(true);
	}

	if (settings->customColors()) {
		enableCustomColorsRadioButton->setChecked(true);
	} else {
		disableCustomColorsRadioButton->setChecked(true);
	}
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
	QString path = QFileDialog::getOpenFileName(this, tr("Choose your custom icon"), QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).first(), tr("Pictures (*.jpg *.jpeg *.png)"));

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
