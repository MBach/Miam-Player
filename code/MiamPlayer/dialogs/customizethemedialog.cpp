#include "customizethemedialog.h"

#include <QDesktopWidget>
#include <QFileDialog>
#include <QScrollBar>
#include <QStandardPaths>

#include <QGraphicsOpacityEffect>

#include "mainwindow.h"
#include "settings.h"

#include "starrating.h"

CustomizeThemeDialog::CustomizeThemeDialog(QWidget *parent) :
	QDialog(nullptr), _targetedColor(nullptr), _animation(new QPropertyAnimation(this, "windowOpacity")), _timer(new QTimer(this))
{
	setupUi(this);

	listWidget->setAttribute(Qt::WA_MacShowFocusRect, false);

	this->setWindowFlags(Qt::Tool);
	this->setAttribute(Qt::WA_DeleteOnClose);

	mainWindow = qobject_cast<MainWindow *>(parent);
	buttonsListBox->setVisible(false);

	spinBoxLibrary->setMouseTracking(true);

	// Animates this Dialog
	_timer->setInterval(3000);
	_timer->setSingleShot(true);
	_animation->setDuration(200);
	_animation->setTargetObject(this);

	this->setupActions();

	SettingsPrivate *settings = SettingsPrivate::instance();
	this->restoreGeometry(settings->value("customizeThemeDialogGeometry").toByteArray());
	listWidget->setCurrentRow(settings->value("customizeThemeDialogCurrentTab").toInt());
}

void CustomizeThemeDialog::setupActions()
{
	SettingsPrivate *settings = SettingsPrivate::instance();
	for (MediaButton *b : mainWindow->mediaButtons) {
		connect(sizeButtonsSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), b, &MediaButton::setSize);
	}

	// Select button theme and size
	connect(themeComboBox, static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged), this, &CustomizeThemeDialog::setThemeNameAndDialogButtons);
	connect(customizeThemeCheckBox, &QCheckBox::toggled, this, [=](bool b) {
		settings->setThemeCustomized(b);
		if (!b) {
			// Restore all buttons when unchecked
			for (QCheckBox *button : customizeButtonsScrollArea->findChildren<QCheckBox*>()) {
				if (!button->isChecked()) {
					button->toggle();
				}
			}
			// Restore default image
			for (MediaButton *b : mainWindow->mediaButtons) {
				b->setIcon(QIcon());
			}
		}
	});
	connect(sizeButtonsSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), settings, &SettingsPrivate::setButtonsSize);

	// Hide buttons or not
	for (MediaButton *b : mainWindow->mediaButtons) {
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
		for(MediaButton *b : mainWindow->mediaButtons) {
			b->setFlat(isFlat);
		}
	});

	// Connect a file dialog to every button if one wants to customize everything
	for (QPushButton *pushButton : customizeButtonsScrollArea->findChildren<QPushButton*>()) {
		connect(pushButton, &QPushButton::clicked, this, &CustomizeThemeDialog::openChooseIconDialog);
	}

	// Extended Search Area
	connect(radioButtonShowExtendedSearch, &QRadioButton::toggled, settings, &SettingsPrivate::setExtendedSearchVisible);

	// Volume bar
	connect(radioButtonShowVolume, &QRadioButton::toggled, this, [=](bool b) {
		settings->setVolumeBarTextAlwaysVisible(b);
		mainWindow->volumeSlider->update();
	});
	connect(spinBoxHideVolumeLabel, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), settings, &SettingsPrivate::setVolumeBarHideAfter);

	// Fonts
	connect(fontComboBoxPlaylist, &QFontComboBox::currentFontChanged, [=](const QFont &font) {
		settings->setFont(SettingsPrivate::FF_Playlist, font);
		mainWindow->tabPlaylists->updateRowHeight();
		for (Playlist *playlist : mainWindow->tabPlaylists->playlists()) {
			for (int i = 0; i < playlist->model()->columnCount(); i++) {
				playlist->model()->setHeaderData(i, Qt::Horizontal, font, Qt::FontRole);
			}
		}
		this->fade();
	});
	connect(fontComboBoxLibrary, &QFontComboBox::currentFontChanged, [=](const QFont &font) {
		settings->setFont(SettingsPrivate::FF_Library, font);
		this->fade();
		mainWindow->library->viewport()->update();
	});
	connect(fontComboBoxMenus, &QFontComboBox::currentFontChanged, [=](const QFont &font) {
		settings->setFont(SettingsPrivate::FF_Menu, font);
		this->fade();
		mainWindow->updateFonts(font);
	});

	// And fonts size
	connect(spinBoxPlaylist, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](int i) {
		settings->setFontPointSize(SettingsPrivate::FF_Playlist, i);
		mainWindow->tabPlaylists->updateRowHeight();
		for (Playlist *playlist : mainWindow->tabPlaylists->playlists()) {
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
	for (QToolButton *b : groupBoxCustomColors->findChildren<QToolButton*>()) {
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

	// Change cover size
	QTimer *reloadCoverSizeTimer = new QTimer(this);
	reloadCoverSizeTimer->setSingleShot(true);
	connect(spinBoxCoverSize, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](int cs) {
		settings->setCoverSize(cs);
		if (mainWindow->library->model()) {
			mainWindow->library->model()->layoutChanged();
			this->fade();
		}
		reloadCoverSizeTimer->start(1000);
	});
	connect(reloadCoverSizeTimer, &QTimer::timeout, mainWindow->library, &LibraryTreeView::reloadCovers);

	// Change big cover opacity
	connect(radioButtonEnableBigCover, &QRadioButton::toggled, [=](bool b) {
		settings->setBigCovers(b);
		labelBigCoverOpacity->setEnabled(b);
		spinBoxBigCoverOpacity->setEnabled(b);
		this->fade();
		mainWindow->library->viewport()->repaint();
	});
	connect(spinBoxBigCoverOpacity, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](int v) {
		settings->setBigCoverOpacity(v);
		this->fade();
		mainWindow->library->viewport()->repaint();
	});

	// Filter library
	connect(radioButtonEnableArticles, &QRadioButton::toggled, this, [=](bool b) {
		settings->setIsLibraryFilteredByArticles(b);
		// Don't reorder the library if one hasn't typed an article yet
		if (!settings->libraryFilteredByArticles().isEmpty()) {
			mainWindow->library->model()->rebuildSeparators();
		}
	});
	connect(articlesLineEdit, &CustomizeThemeTagLineEdit::taglistHasChanged, this, [=](const QStringList &articles) {
		settings->setLibraryFilteredByArticles(articles);
		mainWindow->library->model()->rebuildSeparators();
	});
	connect(radioButtonEnableReorderArtistsArticle, &QRadioButton::toggled, this, [=](bool b) {
		settings->setReorderArtistsArticle(b);
		this->fade();
		mainWindow->library->viewport()->repaint();
	});

	// Tabs
	connect(radioButtonTabsRect, &QRadioButton::toggled, [=](bool b) {
		settings->setTabsRect(b);
		this->fade();
		mainWindow->tabPlaylists->tabBar()->update();
		mainWindow->tabPlaylists->cornerWidget(Qt::TopRightCorner)->update();
	});
	connect(overlapTabsSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](int v) {
		settings->setTabsOverlappingLength(v);
		this->fade();
		mainWindow->tabPlaylists->tabBar()->update();
	});

	// Star delegates
	connect(radioButtonEnableStarDelegate, &QRadioButton::toggled, this, [=](bool b) {
		settings->setDelegates(b);
		labelLibraryDelegates->setEnabled(b);
		radioButtonShowNeverScoredTracks->setEnabled(b);
		radioButtonHideNeverScoredTracks->setEnabled(b);
	});

	connect(radioButtonShowNeverScoredTracks, &QRadioButton::toggled, settings, &SettingsPrivate::setShowNeverScored);
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
	SettingsPrivate *settings = SettingsPrivate::instance();
	settings->setValue("customizeThemeDialogGeometry", saveGeometry());
	settings->setValue("customizeThemeDialogCurrentTab", listWidget->currentRow());

	QDialog::closeEvent(e);
}

void CustomizeThemeDialog::showEvent(QShowEvent *event)
{
	QDialog::showEvent(event);

	/// XXX: why should I show the dialog before adding tags to have the exact and right size?
	/// Is it impossible to compute real size even if dialog is hidden?
	// Add grammatical articles
	for (QString article : SettingsPrivate::instance()->libraryFilteredByArticles()) {
		articlesLineEdit->addTag(article);
	}
	this->activateWindow();
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
		qDebug() << _targetedColor->objectName() << _targetedColor->color();
		_targetedColor->setColor(SettingsPrivate::instance()->customColors(_targetedColor->colorRole()));
		qDebug() << _targetedColor->objectName() << _targetedColor->color();
		ColorDialog *colorDialog = new ColorDialog(this);
		colorDialog->setCurrentColor(_targetedColor->color());
		//qDebug() << colorDialog->currentColor() << _targetedColor->color();
		this->hide();
		colorDialog->exec();
	}
}

void CustomizeThemeDialog::toggleAlternativeBackgroundColor(bool b)
{
	SettingsPrivate::instance()->setColorsAlternateBG(b);
	for (int i = 0; i < mainWindow->tabPlaylists->count(); i++) {
		Playlist *p = mainWindow->tabPlaylists->playlist(i);
		p->setAlternatingRowColors(b);
	}
}

void CustomizeThemeDialog::toggleCustomColors(bool b)
{
	qDebug() << Q_FUNC_INFO << b;
	SettingsPrivate *settings = SettingsPrivate::instance();
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
		qDebug() << Q_FUNC_INFO << settings->customColors(QPalette::Base) << settings->customColors(QPalette::Highlight);

		bgPrimaryColorWidget->setColor(settings->customColors(QPalette::Base));
		selectedItemColorWidget->setColor(settings->customColors(QPalette::Highlight));
	} else {
		QColor base = style()->standardPalette().base().color();
		QColor highlight = style()->standardPalette().highlight().color();
		int gray = qGray(base.rgb());
		bgPrimaryColorWidget->setColor(QColor(gray, gray, gray));
		gray = qGray(highlight.rgb());
		selectedItemColorWidget->setColor(QColor(gray, gray, gray));
		QApplication::setPalette(style()->standardPalette());
	}
}

/** Load theme at startup. */
void CustomizeThemeDialog::loadTheme()
{
	SettingsPrivate *settings = SettingsPrivate::instance();
	customizeThemeCheckBox->setChecked(settings->isThemeCustomized());

	sizeButtonsSpinBox->setValue(settings->buttonsSize());
	flatButtonsCheckBox->setChecked(settings->buttonsFlat());

	// Select the right drop-down item according to the theme
	int i = 0;
	while (Settings::instance()->theme() != themeComboBox->itemText(i).toLower()) {
		i++;
	}
	themeComboBox->setCurrentIndex(i);

	// Buttons
	for (MediaButton *b : mainWindow->mediaButtons) {

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
	spinBoxCoverSize->blockSignals(true);
	spinBoxCoverSize->setValue(settings->coverSize());
	spinBoxCoverSize->blockSignals(false);

	// Colors
	settings->colorsAlternateBG() ? enableAlternateBGRadioButton->setChecked(true) : disableAlternateBGRadioButton->setChecked(true);
	settings->isCustomColors() ? enableCustomColorsRadioButton->setChecked(true) : disableCustomColorsRadioButton->setChecked(true);
	this->toggleCustomColors(settings->isCustomColors());

	// Covers
	radioButtonEnableBigCover->setChecked(settings->isBigCoverEnabled());
	spinBoxBigCoverOpacity->setValue(settings->bigCoverOpacity() * 100);

	// Tabs
	radioButtonTabsRect->setChecked(settings->isRectTabs());
	overlapTabsSpinBox->setValue(settings->tabsOverlappingLength());

	// Articles
	radioButtonEnableArticles->blockSignals(true);
	bool isFiltered = settings->isLibraryFilteredByArticles();
	radioButtonEnableArticles->setChecked(isFiltered);
	std::list<QWidget*> enabled = { articlesLineEdit, labelReorderArtistsArticle, labelReorderArtistsArticleExample, radioButtonEnableReorderArtistsArticle, radioButtonDisableReorderArtistsArticle };
	for (QWidget *w : enabled) {
		w->setEnabled(isFiltered);
	}
	radioButtonEnableReorderArtistsArticle->setChecked(settings->isReorderArtistsArticle());
	radioButtonEnableArticles->blockSignals(false);

	// Star delegate
	if (settings->isStarDelegates()) {
		radioButtonEnableStarDelegate->setChecked(true);
	} else {
		radioButtonDisableStarDelegate->setChecked(true);
	}
	if (settings->isShowNeverScored()) {
		radioButtonShowNeverScoredTracks->setChecked(true);
	} else {
		radioButtonHideNeverScoredTracks->setChecked(true);
	}
}

/** Redefined to initialize favorites from settings. */
int CustomizeThemeDialog::exec()
{
	// Change the label that talks about star delegates
	SettingsPrivate *settings = SettingsPrivate::instance();
	bool starDelegateState = settings->isStarDelegates();
	labelLibraryDelegates->setEnabled(starDelegateState);
	radioButtonShowNeverScoredTracks->setEnabled(starDelegateState);
	radioButtonHideNeverScoredTracks->setEnabled(starDelegateState);

	if (settings->value("customizeThemeDialogGeometry").isNull()) {
		int w = qApp->desktop()->screenGeometry().width() / 2;
		int h = qApp->desktop()->screenGeometry().height() / 2;
		this->move(w - frameGeometry().width() / 2, h - frameGeometry().height() / 2);
	}
	return QDialog::exec();
}

void CustomizeThemeDialog::openChooseIconDialog()
{
	QPushButton *button = qobject_cast<QPushButton *>(sender());
	SettingsPrivate *settings = SettingsPrivate::instance();

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

	for (MediaButton *b : mainWindow->findChildren<MediaButton*>(button->objectName() + "Button")) {
		if (b) {
			b->setIcon(QIcon(path));
		}
	}

	MediaButton *b = mainWindow->findChild<MediaButton*>(button->objectName() + "Button");
	if (path.isEmpty()) {
		button->setIcon(QIcon(":/player/" + Settings::instance()->theme() + "/" + button->objectName()));
		b->setIcon(QIcon());
	} else {
		settings->setValue("customIcons/lastOpenPath", QFileInfo(path).absolutePath());
		button->setIcon(QIcon(path));
		b->setIcon(QIcon(path));
	}
}

/** Changes the current theme and updates this dialog too. */
void CustomizeThemeDialog::setThemeNameAndDialogButtons(QString newTheme)
{
	SettingsPrivate *settings = SettingsPrivate::instance();
	// Check for each button if there is a custom icon
	for (QPushButton *button : customizeButtonsScrollArea->findChildren<QPushButton*>()) {
		if (button) {
			// Keep the custom icon provided by one
			if (settings->hasCustomIcon(button->objectName())) {
				button->setIcon(QIcon(settings->customIcon(button->objectName())));
			} else {
				button->setIcon(QIcon(":/player/" + newTheme.toLower() + "/" + button->objectName()));
			}
		}
	}
	Settings::instance()->setThemeName(newTheme);
	for (MediaButton *m : mainWindow->mediaButtons) {
		m->setIconFromTheme(newTheme);
	}
}
