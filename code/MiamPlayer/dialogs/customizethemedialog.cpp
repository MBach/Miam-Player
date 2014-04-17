#include "customizethemedialog.h"

#include <QDesktopWidget>
#include <QFileDialog>
#include <QScrollBar>
#include <QStandardPaths>

#include <QGraphicsOpacityEffect>

#include "mainwindow.h"

CustomizeThemeDialog::CustomizeThemeDialog(QWidget *parent) :
	QDialog(parent), _targetedColor(NULL)
{
	setupUi(this);
	this->setWindowFlags(Qt::Tool);
	this->setModal(true);

	mainWindow = qobject_cast<MainWindow *>(parent);
	buttonsListBox->setVisible(false);

	spinBoxLibrary->installEventFilter(this);
	spinBoxPlaylist->installEventFilter(this);
	fontComboBoxLibrary->installEventFilter(this);
	fontComboBoxPlaylist->installEventFilter(this);

	spinBoxLibrary->setMouseTracking(true);

	// Animates this Dialog
	_timer = new QTimer(this);
	_timer->setInterval(3000);
	_timer->setSingleShot(true);
	_animation = new QPropertyAnimation(this, "windowOpacity");
	_animation->setDuration(200);
	_animation->setTargetObject(this);

	this->setupActions();
}

void CustomizeThemeDialog::setupActions()
{
	Settings *settings = Settings::getInstance();
	foreach(MediaButton *b, mainWindow->mediaButtons) {
		connect(sizeButtonsSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), b, &MediaButton::setSize);
	}

	// Select button theme and size
	connect(themeComboBox, static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged), this, &CustomizeThemeDialog::setThemeNameAndDialogButtons);
	connect(sizeButtonsSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), settings, &Settings::setButtonsSize);

	// Hide buttons or not
	foreach(MediaButton *b, mainWindow->mediaButtons) {
		QCheckBox *checkBox = findChild<QCheckBox *>(b->objectName().replace("Button", "CheckBox"));
		if (checkBox) {
			connect(checkBox, &QCheckBox::toggled, [=] (bool visible) {
				b->setVisible(visible && !mainWindow->tagEditor->isVisible());
				settings->setMediaButtonVisible(b->objectName(), visible);
			});
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

	connect(flatButtonsCheckBox, &QCheckBox::toggled, settings, &Settings::setButtonsFlat);

	// Fonts	
	connect(fontComboBoxPlaylist, &QFontComboBox::currentFontChanged, [=](const QFont &font) {
		settings->setFont(Settings::PLAYLIST, font);
		mainWindow->tabPlaylists->updateRowHeight();
		foreach (Playlist *playlist, mainWindow->tabPlaylists->playlists()) {
			for (int i = 0; i < playlist->model()->columnCount(); i++) {
				playlist->model()->setHeaderData(i, Qt::Horizontal, font, Qt::FontRole);
			}
		}
		this->fade();
	});
	connect(fontComboBoxLibrary, &QFontComboBox::currentFontChanged, [=](const QFont &font) {
		settings->setFont(Settings::LIBRARY, font);
		this->fade();
	});
	connect(fontComboBoxMenus, &QFontComboBox::currentFontChanged, [=](const QFont &font) {
		settings->setFont(Settings::MENUS, font);
		mainWindow->updateFonts(font);
		this->fade();
	});

	// And fonts size
	connect(spinBoxPlaylist, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](int i) {
		settings->setFontPointSize(Settings::PLAYLIST, i);
		mainWindow->tabPlaylists->updateRowHeight();
		foreach (Playlist *playlist, mainWindow->tabPlaylists->playlists()) {
			for (int i = 0; i < playlist->model()->columnCount(); i++) {
				playlist->model()->setHeaderData(i, Qt::Horizontal, settings->font(Settings::PLAYLIST), Qt::FontRole);
			}
		}
		this->fade();
	});
	connect(spinBoxLibrary, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](int i) {
		settings->setFontPointSize(Settings::LIBRARY, i);
		mainWindow->library->model()->layoutChanged();
		QFont lowerFont = settings->font(Settings::LIBRARY);
		lowerFont.setPointSize(lowerFont.pointSizeF() * 0.7);
		mainWindow->library->model()->setHeaderData(0, Qt::Horizontal, lowerFont, Qt::FontRole);
		this->fade();
	});
	connect(spinBoxMenus, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](int i) {
		settings->setFontPointSize(Settings::MENUS, i);
		mainWindow->updateFonts(settings->font(Settings::MENUS));
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

	// Library
	connect(checkBoxDisplayCovers, &QCheckBox::toggled, [=](bool b) {
		settings->setCovers(b);
		//mainWindow->library->reset();
	});

	// Covers
	connect(spinBoxCoverSize, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](int cs) {
		settings->setCoverSize(cs);
	});
	connect(radioButtonEnableBigCover, &QRadioButton::toggled, [=](bool b) {
		settings->setBigCovers(b);
		labelBigCoverOpacity->setEnabled(b);
		spinBoxBigCoverOpacity->setEnabled(b);
	});
	connect(spinBoxBigCoverOpacity, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [=](int v) {
		settings->setBigCoverOpacity(v);
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
	/*if (event->type() == QEvent::FocusOut) {
		if (_timer->isActive()) {
			_timer->stop();
			this->animate(0.5, 1.0);
		}
		event->accept();
	} else
	if (event->type() == QEvent::MouseMove) {
		QWidget *childWidget = focusWidget();
		//if (childWidget->parent() == obj) {
			QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
			QPoint point = childWidget->mapToParent(childWidget->rect().center()) - mouseEvent->pos();
			qDebug() << point.manhattanLength() << point << childWidget->rect().center();
			//if (childWidget->hasFocus() && _timer->isActive() && point.manhattanLength() > 100) {
			//qDebug() << "stop animation" << childWidget->objectName();
			//	_timer->stop();
			//	this->animate(0.5, 1.0);
			//}
		//}
		if (childWidget->hasFocus() && _timer->isActive() && point.manhattanLength() > 300) {
			_timer->stop();
			this->animate(0.5, 1.0);
		}
	}*/
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
	Settings *settings = Settings::getInstance();
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
	Settings::getInstance()->setColorsAlternateBG(b);
	for (int i = 0; i < mainWindow->tabPlaylists->count() - 1; i++) {
		Playlist *p = mainWindow->tabPlaylists->playlist(i);
		p->setAlternatingRowColors(b);
	}
}

void CustomizeThemeDialog::toggleCustomColors(bool b)
{
	Settings *settings = Settings::getInstance();
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
	Settings *settings = Settings::getInstance();
	sizeButtonsSpinBox->setValue(settings->buttonsSize());
	flatButtonsCheckBox->setChecked(settings->buttonsFlat());

	// Select the right drop-down item according to the theme
	int i = 0;
	while (settings->theme() != themeComboBox->itemText(i).toLower()) {
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

	// Fonts
	fontComboBoxPlaylist->setCurrentFont(settings->font(Settings::PLAYLIST));
	fontComboBoxLibrary->setCurrentFont(settings->font(Settings::LIBRARY));
	fontComboBoxMenus->setCurrentFont(settings->font(Settings::MENUS));
	spinBoxPlaylist->setValue(settings->fontSize(Settings::PLAYLIST));
	spinBoxLibrary->blockSignals(true);
	spinBoxLibrary->setValue(settings->fontSize(Settings::LIBRARY));
	spinBoxLibrary->blockSignals(false);
	spinBoxMenus->setValue(settings->fontSize(Settings::MENUS));

	// Library
	checkBoxDisplayCovers->setChecked(settings->isCoversEnabled());
	spinBoxCoverSize->setValue(settings->coverSize());

	// Colors
	if (settings->colorsAlternateBG()) {
		enableAlternateBGRadioButton->setChecked(true);
	} else {
		disableAlternateBGRadioButton->setChecked(true);
	}

	if (settings->isCustomColors()) {
		enableCustomColorsRadioButton->setChecked(true);
	} else {
		disableCustomColorsRadioButton->setChecked(true);
	}
	this->toggleCustomColors(settings->isCustomColors());

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
	this->activateWindow();
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
