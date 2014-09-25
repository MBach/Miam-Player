#include "customizethemedialog.h"

#include <QDesktopWidget>
#include <QFileDialog>
#include <QScrollBar>
#include <QStandardPaths>

#include <QGraphicsOpacityEffect>

#include "mainwindow.h"
#include "settings.h"

CustomizeThemeDialog::CustomizeThemeDialog(QWidget *parent) :
	QDialog(parent), _targetedColor(NULL)
{
	setupUi(this);
	articlesLineEdit->setAutoTransform(true);
	this->setWindowFlags(Qt::Tool);
	this->setModal(true);

	mainWindow = qobject_cast<MainWindow *>(parent);
	buttonsListBox->setVisible(false);

	spinBoxLibrary->setMouseTracking(true);

	// Animates this Dialog
	_timer = new QTimer(this);
	_timer->setInterval(3000);
	_timer->setSingleShot(true);
	_animation = new QPropertyAnimation(this, "windowOpacity");
	_animation->setDuration(200);
	_animation->setTargetObject(this);

	this->setupActions();

	SettingsPrivate *settings = SettingsPrivate::getInstance();
	this->restoreGeometry(settings->value("customizeThemeDialogGeometry").toByteArray());
	listWidget->setCurrentRow(settings->value("customizeThemeDialogCurrentTab").toInt());
}

void CustomizeThemeDialog::setupActions()
{
	SettingsPrivate *settings = SettingsPrivate::getInstance();
	foreach(MediaButton *b, mainWindow->mediaButtons) {
		connect(sizeButtonsSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), b, &MediaButton::setSize);
	}

	// Select button theme and size
	connect(themeComboBox, static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged), this, &CustomizeThemeDialog::setThemeNameAndDialogButtons);
	connect(customizeThemeCheckBox, &QCheckBox::toggled, this, [=](bool b) {
		settings->setThemeCustomized(b);
		if (!b) {
			// Restore all buttons when unchecked
			foreach (QCheckBox *button, customizeButtonsScrollArea->findChildren<QCheckBox*>()) {
				if (!button->isChecked()) {
					button->toggle();
				}
			}
		}
	});
	connect(sizeButtonsSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), settings, &SettingsPrivate::setButtonsSize);

	// Hide buttons or not
	foreach(MediaButton *b, mainWindow->mediaButtons) {
		QCheckBox *checkBox = findChild<QCheckBox *>(b->objectName().replace("Button", "CheckBox"));
		if (checkBox) {
			connect(checkBox, &QCheckBox::toggled, [=] (bool visible) {
				b->setVisible(visible && !mainWindow->tagEditor->isVisible());
				settings->setMediaButtonVisible(b->objectName(), visible);
			});
		}
	}

	// Make buttons flat
	connect(flatButtonsCheckBox, &QCheckBox::toggled, this, [=] (bool isFlat) {
		settings->setButtonsFlat(isFlat);
		foreach(MediaButton *b, mainWindow->mediaButtons) {
			b->setFlat(isFlat);
		}
	});

	// Connect a file dialog to every button if one wants to customize everything
	foreach (QPushButton *pushButton, customizeButtonsScrollArea->findChildren<QPushButton*>()) {
		connect(pushButton, &QPushButton::clicked, this, &CustomizeThemeDialog::openChooseIconDialog);
	}

	// Extended Search Area
	connect(radioButtonShowExtendedSearch, &QRadioButton::toggled, settings, &SettingsPrivate::setExtendedSearchVisible);

	// Volume bar
	connect(radioButtonShowVolume, &QRadioButton::toggled, settings, &SettingsPrivate::setVolumeBarTextAlwaysVisible);
	connect(spinBoxHideVolumeLabel, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), settings, &SettingsPrivate::setVolumeBarHideAfter);

	// Fonts
	connect(fontComboBoxPlaylist, &QFontComboBox::currentFontChanged, [=](const QFont &font) {
		settings->setFont(SettingsPrivate::FF_Playlist, font);
		mainWindow->tabPlaylists->updateRowHeight();
		foreach (Playlist *playlist, mainWindow->tabPlaylists->playlists()) {
			for (int i = 0; i < playlist->model()->columnCount(); i++) {
				playlist->model()->setHeaderData(i, Qt::Horizontal, font, Qt::FontRole);
			}
		}
		this->fade();
	});
	connect(fontComboBoxLibrary, &QFontComboBox::currentFontChanged, [=](const QFont &font) {
		settings->setFont(SettingsPrivate::FF_Library, font);
		this->fade();
	});
	connect(fontComboBoxMenus, &QFontComboBox::currentFontChanged, [=](const QFont &font) {
		settings->setFont(SettingsPrivate::FF_Menu, font);
		mainWindow->updateFonts(font);
		this->fade();
	});

	// And fonts size
	connect(spinBoxPlaylist, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](int i) {
		settings->setFontPointSize(SettingsPrivate::FF_Playlist, i);
		mainWindow->tabPlaylists->updateRowHeight();
		foreach (Playlist *playlist, mainWindow->tabPlaylists->playlists()) {
			for (int i = 0; i < playlist->model()->columnCount(); i++) {
				playlist->model()->setHeaderData(i, Qt::Horizontal, settings->font(SettingsPrivate::FF_Playlist), Qt::FontRole);
			}
		}
		this->fade();
	});
	connect(spinBoxLibrary, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](int i) {
		settings->setFontPointSize(SettingsPrivate::FF_Library, i);
		mainWindow->library->model()->layoutChanged();
		QFont lowerFont = settings->font(SettingsPrivate::FF_Library);
		lowerFont.setPointSize(lowerFont.pointSizeF() * 0.7);
		mainWindow->library->model()->setHeaderData(0, Qt::Horizontal, lowerFont, Qt::FontRole);
		this->fade();
	});
	connect(spinBoxMenus, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](int i) {
		settings->setFontPointSize(SettingsPrivate::FF_Menu, i);
		mainWindow->updateFonts(settings->font(SettingsPrivate::FF_Menu));
		this->fade();
	});

	// Timer
	connect(_timer, &QTimer::timeout, [=]() { this->animate(0.5, 1.0); });

	// Colors
	connect(enableCustomColorsRadioButton, &QCheckBox::toggled, this, &CustomizeThemeDialog::toggleCustomColors);
	connect(enableAlternateBGRadioButton, &QRadioButton::toggled, this, &CustomizeThemeDialog::toggleAlternativeBackgroundColor);
	foreach (QToolButton *b, groupBoxCustomColors->findChildren<QToolButton*>()) {
		connect(b, &QToolButton::clicked, this, &CustomizeThemeDialog::showColorDialog);
	}

	// Show covers in the Library
	connect(checkBoxDisplayCovers, &QCheckBox::toggled, [=](bool b) {
		settings->setCovers(b);
		if (mainWindow->library->model()) {
			mainWindow->library->model()->layoutChanged();
			this->fade();
		}
	});

	// Filter library
	connect(radioButtonEnableArticles, &QRadioButton::toggled, this, [=](bool b) {
		settings->setIsLibraryFilteredByArticles(b);
		// Don't reorder the library if one hasn't typed an article yet
		if (b || (!b && !settings->libraryFilteredByArticles().isEmpty())) {
			mainWindow->library->changeHierarchyOrder();
		}
	});
	connect(articlesLineEdit, &TagLineEdit::taglistHasChanged, this, [=](const QStringList &articles) {
		settings->setLibraryFilteredByArticles(articles);
		mainWindow->library->changeHierarchyOrder();
	});

	// Change cover size
	connect(spinBoxCoverSize, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](int cs) {
		settings->setCoverSize(cs);
		if (mainWindow->library->model()) {
			mainWindow->library->model()->layoutChanged();
			this->fade();
		}
	});
	connect(radioButtonEnableBigCover, &QRadioButton::toggled, [=](bool b) {
		settings->setBigCovers(b);
		labelBigCoverOpacity->setEnabled(b);
		spinBoxBigCoverOpacity->setEnabled(b);
	});
	connect(spinBoxBigCoverOpacity, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](int v) {
		settings->setBigCoverOpacity(v);
		this->fade();
		mainWindow->repaint();
	});

	// Tabs
	connect(radioButtonTabsRect, &QRadioButton::toggled, [=](bool b) {
		settings->setTabsRect(b);
		this->fade();
		mainWindow->repaint();
	});
	connect(overlapTabsSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](int v) {
		settings->setTabsOverlappingLength(v);
		this->fade();
		mainWindow->repaint();
	});
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

	SettingsPrivate *settings = SettingsPrivate::getInstance();
	settings->setValue("customizeThemeDialogGeometry", saveGeometry());
	settings->setValue("customizeThemeDialogCurrentTab", listWidget->currentRow());

	QDialog::closeEvent(e);
}

/** Automatically centers the parent window when closing this dialog. */
void CustomizeThemeDialog::mouseMoveEvent(QMouseEvent *event)
{
	qDebug() << event->pos();
	QDialog::mouseMoveEvent(event);
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
	SettingsPrivate *settings = SettingsPrivate::getInstance();
	_targetedColor = findChild<Reflector*>(sender()->objectName().replace("ToolButton", "Widget"));
	if (_targetedColor) {
		ColorDialog *colorDialog = new ColorDialog(this);
		colorDialog->show();
		colorDialog->setCurrentColor(_targetedColor->color());

		connect(colorDialog, &ColorDialog::currentColorChanged, [=] (const QColor &selectedColor) {
			settings->setCustomColorRole(_targetedColor->colorRole(), selectedColor);
		});
		connect(colorDialog, &ColorDialog::aboutToBeClosed, [=] () {
			_targetedColor->setColor(colorDialog->currentColor());
		});

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

void CustomizeThemeDialog::toggleAlternativeBackgroundColor(bool b)
{
	SettingsPrivate::getInstance()->setColorsAlternateBG(b);
	for (int i = 0; i < mainWindow->tabPlaylists->count() - 1; i++) {
		Playlist *p = mainWindow->tabPlaylists->playlist(i);
		p->setAlternatingRowColors(b);
	}
}

void CustomizeThemeDialog::toggleCustomColors(bool b)
{
	SettingsPrivate *settings = SettingsPrivate::getInstance();
	settings->setCustomColors(b);
	for (int i = 0; i < customColorsGridLayout->rowCount(); i++) {
		for (int j = 0; j < customColorsGridLayout->columnCount(); j++) {
			QLayoutItem *item = customColorsGridLayout->itemAtPosition(i, j);
			if (item->widget()) {
				item->widget()->setEnabled(b);
			}
		}
	}
	if (b) {
		bgPrimaryColorWidget->setColor(settings->customColors(QPalette::Base));
		selectedItemColorWidget->setColor(settings->customColors(QPalette::Highlight));
	} else {
		int gray = qGray(settings->customColors(QPalette::Base).rgb());
		bgPrimaryColorWidget->setColor(QColor(gray, gray, gray));
		gray = qGray(settings->customColors(QPalette::Highlight).rgb());
		selectedItemColorWidget->setColor(QColor(gray, gray, gray));
	}
}

/** Load theme at startup. */
void CustomizeThemeDialog::loadTheme()
{
	SettingsPrivate *settings = SettingsPrivate::getInstance();
	customizeThemeCheckBox->setChecked(settings->isThemeCustomized());

	sizeButtonsSpinBox->setValue(settings->buttonsSize());
	flatButtonsCheckBox->setChecked(settings->buttonsFlat());

	// Select the right drop-down item according to the theme
	int i = 0;
	while (Settings::getInstance()->theme() != themeComboBox->itemText(i).toLower()) {
		i++;
	}
	themeComboBox->setCurrentIndex(i);

	// Buttons
	foreach(MediaButton *b, mainWindow->mediaButtons) {
		b->setVisible(settings->isMediaButtonVisible(b->objectName()));

		// Check or uncheck checkboxes in this customize interface
		QCheckBox *checkBox = findChild<QCheckBox *>(b->objectName().replace("Button", "CheckBox"));
		if (checkBox) {
			checkBox->setChecked(settings->isMediaButtonVisible(b->objectName()));
		}

		// Display customs icons, if any
		QPushButton *pushButton = findChild<QPushButton *>(b->objectName().remove("Button"));
		if (pushButton) {
			pushButton->setIcon(b->icon());
		}
	}

	// Extended Search Area
	settings->isExtendedSearchVisible() ? radioButtonShowExtendedSearch->setChecked(true) : radioButtonHideExtendedSearch->setChecked(true);

	// Volume bar
	radioButtonShowVolume->setChecked(settings->isVolumeBarTextAlwaysVisible());
	spinBoxHideVolumeLabel->setValue(settings->volumeBarHideAfter());

	// Fonts
	fontComboBoxPlaylist->setCurrentFont(settings->font(SettingsPrivate::FF_Playlist));
	fontComboBoxLibrary->setCurrentFont(settings->font(SettingsPrivate::FF_Library));
	fontComboBoxMenus->setCurrentFont(settings->font(SettingsPrivate::FF_Menu));
	spinBoxPlaylist->setValue(settings->fontSize(SettingsPrivate::FF_Playlist));
	spinBoxLibrary->blockSignals(true);
	spinBoxLibrary->setValue(settings->fontSize(SettingsPrivate::FF_Library));
	spinBoxLibrary->blockSignals(false);
	spinBoxMenus->setValue(settings->fontSize(SettingsPrivate::FF_Menu));

	// Library
	checkBoxDisplayCovers->setChecked(settings->isCoversEnabled());
	spinBoxCoverSize->setValue(settings->coverSize());

	// Colors
	settings->colorsAlternateBG() ? enableAlternateBGRadioButton->setChecked(true) : disableAlternateBGRadioButton->setChecked(true);
	settings->isCustomColors() ? enableCustomColorsRadioButton->setChecked(true) : disableCustomColorsRadioButton->setChecked(true);
	this->toggleCustomColors(settings->isCustomColors());

	// Tabs
	radioButtonTabsRect->setChecked(settings->isRectTabs());
	overlapTabsSpinBox->setValue(settings->tabsOverlappingLength());

	// Articles
	radioButtonEnableArticles->setChecked(settings->isLibraryFilteredByArticles());
}

/** Redefined to initialize favorites from settings. */
void CustomizeThemeDialog::open()
{
	// Change the label that talks about star delegates
	SettingsPrivate *settings = SettingsPrivate::getInstance();
	bool starDelegateState = settings->isStarDelegates();
	labelLibraryDelegates->setEnabled(starDelegateState);
	radioButtonShowNeverScoredTracks->setEnabled(starDelegateState);
	radioButtonHideNeverScoredTracks->setEnabled(starDelegateState);
	if (starDelegateState) {
		labelLibraryDelegatesState->setText(tr("Favorites are currently enabled"));
	} else {
		labelLibraryDelegatesState->setText(tr("Favorites are currently disabled"));
	}

	if (settings->value("customizeThemeDialogGeometry").isNull()) {
		int w = qApp->desktop()->screenGeometry().width() / 2;
		int h = qApp->desktop()->screenGeometry().height() / 2;
		this->move(w - frameGeometry().width() / 2, h - frameGeometry().height() / 2);
	}
	QDialog::open();
	this->activateWindow();

	/// XXX: why should I show the dialog before adding tags to have the exact and right size?
	/// Is it impossible to compute real size even if dialog is hidden?
	// Add grammatical articles
	foreach (QString article, settings->libraryFilteredByArticles()) {
		articlesLineEdit->addTag(article);
	}
}

void CustomizeThemeDialog::openChooseIconDialog()
{
	QPushButton *button = qobject_cast<QPushButton *>(sender());
	SettingsPrivate *settings = SettingsPrivate::getInstance();

	// It's always more convenient when the dialog re-open at the same location
	QString openPath;
	QVariant variantOpenPath = settings->value("customIcons/lastOpenPath");
	if (variantOpenPath.isValid()) {
		openPath = variantOpenPath.toString();
	} else {
		openPath = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).first();
	}

	QString path = QFileDialog::getOpenFileName(this, tr("Choose your custom icon"), openPath, tr("Pictures (*.jpg *.jpeg *.png)"));

	// Reset custom icon if path is empty (delete key in settings too)
	settings->setCustomIcon(button->objectName() + "Button", path);

	foreach (MediaButton *b, mainWindow->findChildren<MediaButton*>(button->objectName() + "Button")) {
		if (b) {
			b->setIcon(QIcon(path));
		}
	}
	if (path.isEmpty()) {
		button->setIcon(QIcon(":/player/" + Settings::getInstance()->theme() + "/" + button->objectName()));
	} else {
		settings->setValue("customIcons/lastOpenPath", QFileInfo(path).absolutePath());
		button->setIcon(QIcon(path));
	}
}

/** Changes the current theme and updates this dialog too. */
void CustomizeThemeDialog::setThemeNameAndDialogButtons(QString newTheme)
{
	SettingsPrivate *settings = SettingsPrivate::getInstance();
	// Check for each button if there is a custom icon
	foreach(QPushButton *button, customizeButtonsScrollArea->findChildren<QPushButton*>()) {
		if (button) {
			// Keep the custom icon provided by one
			if (settings->hasCustomIcon(button->objectName())) {
				button->setIcon(QIcon(settings->customIcon(button->objectName())));
			} else {
				button->setIcon(QIcon(":/player/" + newTheme.toLower() + "/" + button->objectName()));
			}
		}
	}
	Settings::getInstance()->setThemeName(newTheme);
	qDebug() << Q_FUNC_INFO << "wtf ?";
	foreach(MediaButton *m, mainWindow->mediaButtons) {
		m->setIconFromTheme(newTheme);
	}
}
