#include "customizethemedialog.h"

#include <QDesktopWidget>
#include <QFileDialog>
#include <QScrollBar>
#include <QStandardPaths>

#include <QGraphicsOpacityEffect>

#include "settings.h"

CustomizeThemeDialog::CustomizeThemeDialog(QWidget *parent) :
	QDialog(parent), _targetedColor(NULL)
{
	setupUi(this);
	this->setWindowFlags(Qt::Tool);
	this->setModal(true);

	mainWindow = qobject_cast<MainWindow *>(parent);
	_colorDialog = new ColorDialog(this);
	_styleSheetUpdater = new StyleSheetUpdater(this);
	buttonsListBox->setVisible(false);

	spinBoxLibrary->installEventFilter(this);
	spinBoxPlaylist->installEventFilter(this);
	fontComboBoxLibrary->installEventFilter(this);
	fontComboBoxPlaylist->installEventFilter(this);
	/*spinBoxLibrary->setMouseTracking(true);
	spinBoxPlaylist->setMouseTracking(true);
	fontComboBoxLibrary->setMouseTracking(true);
	fontComboBoxPlaylist->setMouseTracking(true);*/
	//this->installEventFilter(this);
	//this->setMouseTracking(true);

	groupBoxFonts->setMouseTracking(true);
	groupBoxFonts->installEventFilter(this);

	// Animates this Dialog
	_timer = new QTimer(this);
	_timer->setInterval(3000);
	_timer->setSingleShot(true);
	_animation = new QPropertyAnimation(this, "windowOpacity");
	_animation->setDuration(200);
	_animation->setTargetObject(this);


	this->setupActions();
	this->associatePaintableElements();
	this->loadTheme();
}

void CustomizeThemeDialog::associatePaintableElements()
{
	// There are 4 kinds of paintables elements
	bgPrimaryColorWidget->setStyleSheetUpdater(_styleSheetUpdater, StyleSheetUpdater::BACKGROUND);
	globalBackgroundColorWidget->setStyleSheetUpdater(_styleSheetUpdater, StyleSheetUpdater::GLOBAL_BACKGROUND);
	itemColorWidget->setStyleSheetUpdater(_styleSheetUpdater, StyleSheetUpdater::TEXT);
	selectedItemColorWidget->setStyleSheetUpdater(_styleSheetUpdater, StyleSheetUpdater::HOVER);

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
		if (pushButton) {
			connect(flatButtonsCheckBox, &QCheckBox::toggled, [=] (bool flat) { b->setFlat(flat); });
		}
	}
	foreach (QPushButton *pushButton, customizeButtonsScrollArea->findChildren<QPushButton*>()) {
		connect(pushButton, &QPushButton::clicked, this, &CustomizeThemeDialog::openChooseIconDialog);
	}

	connect(flatButtonsCheckBox, SIGNAL(toggled(bool)), settings, SLOT(setButtonsFlat(bool)));

	// Fonts
	foreach (QFontComboBox *fontComboBox, groupBoxFonts->findChildren<QFontComboBox*>()) {
		connect(fontComboBox, &QFontComboBox::currentFontChanged, [=](const QFont &font) {
			if (fontComboBox->objectName().endsWith("Playlist")) {
				settings->setFont(Settings::PLAYLIST, font);
				mainWindow->tabPlaylists->updateRowHeight();
			} else if (fontComboBox->objectName().endsWith("Library")) {
				settings->setFont(Settings::LIBRARY, font);
				mainWindow->library->model()->layoutChanged();
			} else if (fontComboBox->objectName().endsWith("Menus")) {
				settings->setFont(Settings::MENUS, font);
			}
			this->fade();
		});
	}
	// And fonts size
	foreach (QSpinBox *spinBox, groupBoxFonts->findChildren<QSpinBox*>()) {
		connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](int i) {
			if (spinBox->objectName().endsWith("Playlist")) {
				settings->setFontPointSize(Settings::PLAYLIST, i);
				mainWindow->tabPlaylists->updateRowHeight();
			} else {
				settings->setFontPointSize(Settings::LIBRARY, i);
				mainWindow->library->model()->layoutChanged();
			}
			this->fade();
		});
	}

	// Timer
	connect(_timer, &QTimer::timeout, [=]() { this->animate(0.5, 1.0); });

	// Colors
	connect(enableCustomColorsRadioButton, SIGNAL(toggled(bool)), this, SLOT(toggleCustomColors(bool)));
	connect(enableAlternateBGRadioButton, SIGNAL(toggled(bool)), this, SLOT(toggleAlternativeBackgroundColor(bool)));
	foreach (QToolButton *b, findChildren<QToolButton*>()) {
		connect(b, SIGNAL(clicked()), this, SLOT(showColorDialog()));
	}
	connect(_colorDialog, &ColorDialog::currentColorChanged, [=] (const QColor &selectedColor) {
		_styleSheetUpdater->replace(_targetedColor, selectedColor);
	});

	// Library
	connect(checkBoxAlphabeticalSeparators, &QCheckBox::toggled, [=](bool b) {
		settings->setToggleSeparators(b);
		mainWindow->library->beginPopulateTree();
	});
	connect(checkBoxDisplayCovers, &QCheckBox::toggled, [=](bool b) {
		settings->setCovers(b);
		mainWindow->library->beginPopulateTree();
	});

    connect(spinBoxCoverSize, SIGNAL(valueChanged(int)), mainWindow->library, SLOT(setCoverSize(int)));
}

void CustomizeThemeDialog::fade()
{
	if (this->isVisible()) {
		if (!_timer->isActive()) {
			this->animate(1.0, 0.5);
		}
		_timer->start();
	}
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

/** Automatically centers the parent window when closing this dialog. */
void CustomizeThemeDialog::mouseMoveEvent(QMouseEvent *event)
{
	qDebug() << event->pos();
	QDialog::mouseMoveEvent(event);
}

bool CustomizeThemeDialog::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::FocusOut) {
		if (_timer->isActive()) {
			_timer->stop();
			this->animate(0.5, 1.0);
		}
		event->accept();
	} else if (event->type() == QEvent::MouseMove) {
		QWidget *childWidget = focusWidget();
		if (childWidget->parent() == obj) {
			QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
			QPoint point = childWidget->mapToParent(childWidget->rect().center()) - mouseEvent->pos();
			qDebug() << childWidget->mapToParent(childWidget->rect().center()) << mouseEvent->pos() << point.manhattanLength();
		}
		/*if (widget->hasFocus() && _timer->isActive() && point.manhattanLength() > 300) {
			_timer->stop();
			this->animate(0.5, 1.0);
		}*/
	}
	return QDialog::eventFilter(obj, event);
}

void CustomizeThemeDialog::animate(qreal startValue, qreal stopValue)
{
	_animation->setStartValue(startValue);
	_animation->setEndValue(stopValue);
	_animation->start();
}

/** Shows a color dialog and hides this dialog temporarily.
 * Also, reorder the mainWindow and the color dialog to avoid overlapping, if possible. */
void CustomizeThemeDialog::showColorDialog()
{
	_targetedColor = findChild<Reflector*>(sender()->objectName().replace("ToolButton", "Widget"));
	if (_targetedColor) {
		// Very important: gets at runtime the elements that will be repaint by one
		_colorDialog->setPaintableElements(_targetedColor);
		_colorDialog->setCurrentColor(_targetedColor->color());

		// Moves the color dialog right to the mainWindow
		if (parentWidget()->width() + 20 + _colorDialog->width() < qApp->desktop()->availableGeometry().width()) {
			int desktopWidth = qApp->desktop()->availableGeometry().width();
			int w = (desktopWidth - (parentWidget()->width() + 20 + _colorDialog->width())) / 2;
			parentWidget()->move(QPoint(w, parentWidget()->pos().y()));
			int h = parentWidget()->y() + parentWidget()->height() / 2 - _colorDialog->height() / 2;
			_colorDialog->move(parentWidget()->x() + 40 + parentWidget()->width(), h);
		}
		this->hide();
	}
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
		if (pushButton) {
			pushButton->setIcon(b->icon());
		}
	}

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
	if (b) {
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
}

/** Changes the current theme and updates this dialog too. */
void CustomizeThemeDialog::setThemeNameAndDialogButtons(QString newTheme)
{
	Settings *settings = Settings::getInstance();
	// Check for each button if there is a custom icon
	foreach(QPushButton *button, customizeButtonsScrollArea->findChildren<QPushButton*>()) {
		if (button) {
			// Keep the custom icon provided by one
			if (settings->hasCustomIcon(button)) {
				button->setIcon(QIcon(settings->customIcon(button)));
			} else {
				button->setIcon(QIcon(":/player/" + newTheme.toLower() + "/" + button->objectName()));
			}
		}
	}
	settings->setThemeName(newTheme);
	foreach(MediaButton *m, mainWindow->mediaButtons) {
		m->setIconFromTheme(newTheme);
	}
}
