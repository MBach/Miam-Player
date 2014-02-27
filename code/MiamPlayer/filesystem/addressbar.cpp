#include "addressbar.h"

#include <QDir>
#include <QFileIconProvider>
#include <QMouseEvent>
#include <QPainter>
#include <QSpacerItem>

#include <QtDebug>

#include <QLineEdit>

AddressBar::AddressBar(QWidget *parent) :
	QWidget(parent)
{
	_lineEdit = new QLineEdit(this);
	_lineEdit->installEventFilter(this);
	_lineEdit->setFrame(false);

	hBoxLayout = new QHBoxLayout(this);
	this->setContentsMargins(5, 2, 2, 2);
	hBoxLayout->setContentsMargins(0, 0, 0, 0);
	hBoxLayout->setSpacing(0);

	this->setLayout(hBoxLayout);
	this->setMinimumHeight(30);


	this->createRoot();

	menu = new AddressBarMenu(this);
	connect(menu, &QMenu::triggered, this, &AddressBar::appendSubDir);

	this->setMouseTracking(true);
}

bool AddressBar::eventFilter(QObject *obj, QEvent *e)
{
	if (obj == _lineEdit && e->type() == QEvent::FocusOut) {
		qDebug() << "here ?";
		this->createRoot();
		this->init(_lineEdit->text());
		_lineEdit->hide();
	}
	return QWidget::eventFilter(obj, e);
}

void AddressBar::mouseMoveEvent(QMouseEvent *)
{
	this->update();
}

void AddressBar::mousePressEvent(QMouseEvent *)
{
	foreach (AddressBarButton *abb, findChildren<AddressBarButton*>()) {
		abb->hide();
	}
	_lineEdit->setGeometry(this->contentsRect());
	QLayoutItem *item = hBoxLayout->itemAt(hBoxLayout->count() - 2);
	_lineEdit->setText(qobject_cast<AddressBarButton*>(item->widget())->path());
	_lineEdit->selectAll();
	while (hBoxLayout->count() != 0) {
		delete hBoxLayout->takeAt(0);
	}
	hBoxLayout->insertWidget(0, _lineEdit);
	_lineEdit->show();
	_lineEdit->setFocus();
}

void AddressBar::paintEvent(QPaintEvent *)
{
	QPainter p(this);

	// Light gray frame
	QRect r = this->rect();//.adjusted(2, 2, -4, -4);
	p.setPen(QPen(QColor(169, 180, 191), 1.0));
	p.drawRect(r);

	// White rect between frame and blue rect
	p.fillRect(r.adjusted(1, 1, -1, -1), Qt::white);
}

/** Create a special root arrow button.*/
void AddressBar::createRoot()
{
	AddressBarButton *buttonArrow = new AddressBarButton(QDir::separator(), -1, this);
	connect(buttonArrow, &QPushButton::clicked, this, &AddressBar::showDrivesAndPreviousFolders);
	hBoxLayout->insertWidget(0, buttonArrow);
}

/** Append 2 buttons to the address bar to navigate through the filesystem. */
void AddressBar::createSubDirButtons(const QDir &path, bool insertFirst)
{
	AddressBarButton *buttonDir = new AddressBarButton(path.absolutePath(), hBoxLayout->count() - 1, this);
	buttonDir->setIcon(QFileIconProvider().icon(QFileInfo(path.absolutePath())));
	// Special case for the root directory
	if (path.isRoot()) {
		QString root = path.absolutePath();
		if (root.length() > 1) {
			root = root.left(root.length() - 1);
		}
		buttonDir->setText(root);
	} else {
		buttonDir->setText(path.dirName());
	}
	connect(buttonDir, &AddressBarButton::aboutToShowMenu, this, &AddressBar::showSubDirMenu);

	if (insertFirst) {
		hBoxLayout->insertWidget(1, buttonDir);
	} else {
		this->hideFirstButtons(buttonDir);
		hBoxLayout->insertWidget(hBoxLayout->count() - 1, buttonDir);
	}
}

void AddressBar::hideFirstButtons(AddressBarButton *buttonDir)
{
	// Compute the width of the new address bar button before adding
	QFontMetrics fm(buttonDir->font());
	int newButtonWidth = buttonDir->iconSize().width() + 5 + fm.boundingRect(buttonDir->text()).width();
	int spacerWidth = hBoxLayout->itemAt(hBoxLayout->count() - 1)->spacerItem()->geometry().width();

	// If the layout needs to be expanded, then hide first items
	// It can be more than one folder. Furthermore, hide the arrow button
	while (newButtonWidth > spacerWidth) {
		for (int i = 1; i < hBoxLayout->count() - 1; i++) {
			// Hide the folder and its associated button
			if (hBoxLayout->itemAt(i)->widget()->isVisible()) {
				AddressBarButton *previousButton = qobject_cast<AddressBarButton*>(hBoxLayout->itemAt(i)->widget());
				menu->appendSubfolder(previousButton);
				previousButton->hide();
				hBoxLayout->itemAt(++i)->widget()->hide();
				break;
			}
		}
		spacerWidth = hBoxLayout->itemAt(hBoxLayout->count() - 1)->spacerItem()->geometry().width();
		hBoxLayout->activate();
	}
}

void AddressBar::showFirstButtons(AddressBarButton *buttonDir)
{
	// Compute the width of the new address bar button before adding
	QFontMetrics fm(buttonDir->font());
	int newButtonWidth = buttonDir->iconSize().width() + 5 + fm.boundingRect(buttonDir->text()).width();
	int spacerWidth = hBoxLayout->itemAt(hBoxLayout->count() - 1)->spacerItem()->geometry().width();

	// If the layout can be expanded, then show first items (if hidden)
	while (newButtonWidth < spacerWidth) {
		for (int i = hBoxLayout->count() - 2; i > 1; i--) {
			// Hide the folder and its associated button
			if (!hBoxLayout->itemAt(i)->widget()->isVisible()) {
				AddressBarButton *previousButton = qobject_cast<AddressBarButton*>(hBoxLayout->itemAt(i)->widget());
				menu->removeSubfolder(previousButton);
				previousButton->show();
				hBoxLayout->itemAt(--i)->widget()->show();
				break;
			}
		}
		spacerWidth = hBoxLayout->itemAt(hBoxLayout->count() - 1)->spacerItem()->geometry().width();
		hBoxLayout->invalidate();

		// Stop the loop if all widgets are visible
		int maxVisibleWidgets = 0;
		for (int i = 0; i < hBoxLayout->count() - 2; i++) {
			if (hBoxLayout->itemAt(i)->widget()->isVisible()) {
				maxVisibleWidgets++;
			}
		}
		if (maxVisibleWidgets == hBoxLayout->count() - 2) {
			break;
		}
	}
}

/** Init with an absolute path. Also used as a callback to a view. */
void AddressBar::init(const QString &initPath)
{
	this->deleteFromArrowFolder(0);
	QDir dir(initPath);
	while (!dir.isRoot()) {
		this->createSubDirButtons(dir, true);
		dir.cdUp();
	}
	this->createSubDirButtons(dir, true);

	// Re-order index buttons because they were inserted backward (/path/to/music, /path/to, /path, /)
	for (int i = 0; i < hBoxLayout->count() - 1; i++) {
		AddressBarButton *b = qobject_cast<AddressBarButton*>(hBoxLayout->itemAt(i)->widget());
		if (b) {
			b->setIndex(i);
		}
	}
	hBoxLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));
	emit pathChanged(initPath);
}

/** Change the selected path then create subdirectories. */
void AddressBar::appendSubDir(QAction *action)
{
	QList<QWidget*> widgets = action->associatedWidgets();
	if (!widgets.isEmpty()) {
		AddressBarButton *button = NULL;
		// We are sure that there's a not null button
		foreach (QWidget *w, widgets) {
			button = qobject_cast<AddressBarButton*>(w);
			if (button) {
				break;
			}
		}

		// If the sender was the root item
		QDir subDir;
		if (button->index() == 0) {
			subDir.setPath(action->text() + QDir::separator());
		} else {
			subDir.setPath(button->currentPath());
		}
		subDir.cd(action->text() + QDir::separator());
		this->deleteFromArrowFolder(button->index());
		this->createSubDirButtons(subDir);
		emit pathChanged(subDir.absolutePath());
	}
}

/** Delete subdirectories located after the arrow button. */
void AddressBar::deleteFromArrowFolder(int after)
{
	// If we have something to delete after
	if (hBoxLayout->count() - 2 > after) {
		// Delete items from the end (excluding the spacer)
		for (int i = hBoxLayout->count() - 2; i > after; i--) {
			QLayoutItem *item = hBoxLayout->takeAt(i);
			if (item && item->widget()) {
				delete item->widget();
				delete item;
			}
		}

		// Special case for the root button
		if (after == 0) {
			after++;
		}
		AddressBarButton *addressBarButton = qobject_cast<AddressBarButton*>(hBoxLayout->itemAt(after - 1)->widget());
		emit pathChanged(addressBarButton->currentPath());
	}
}

/** Delete subdirectories when one clicks in the middle of this address bar. */
void AddressBar::deleteFromNamedFolder()
{
	// The origin of the click can be a folder or the arrow button just after or a callback function
	if (AddressBarButton *addressBarButton = qobject_cast<AddressBarButton*>(sender())) {
		this->deleteFromArrowFolder(addressBarButton->index() + 1);
		this->showFirstButtons(addressBarButton);
	}
}

/** Show logical drives (on Windows) or root item (on Unix). Also, when the path is too long, first folders are sent to this submenu. */
void AddressBar::showDrivesAndPreviousFolders()
{
	// Delete existing entries
	menu->clear();

	AddressBarButton *firstButton = qobject_cast<AddressBarButton*>(hBoxLayout->itemAt(0)->widget());
	AddressBarButton *nextButton = qobject_cast<AddressBarButton*>(hBoxLayout->itemAt(1)->widget());

	foreach (QFileInfo drive, QDir::drives()) {
		QString driveName = QDir::toNativeSeparators(drive.absoluteFilePath());
		if (driveName.length() > 1) {
			driveName.remove(QDir::separator());
		}
		QAction *action = new QAction(QFileIconProvider().icon(drive), driveName, menu);
		// Check if the new submenu has one of its items already displayed, then make it bold
		if (nextButton != NULL && action->text() == nextButton->text()) {
			QFont font = action->font();
			font.setBold(true);
			action->setFont(font);
		}
		menu->addAction(action);
		firstButton->addAction(action);
	}

	// Then display the menu and the possibly empty list of folders before the first visible folder
	QPoint p(firstButton->geometry().x() - 20, firstButton->geometry().y() + firstButton->geometry().height() - 1);
	menu->exec(mapToGlobal(p), firstButton->index() == 0);
}

/** Show a popup menu with the content of the selected directory. */
void AddressBar::showSubDirMenu()
{
	// Delete existing entries
	menu->clear();

	// Create new entries
	AddressBarButton *button = qobject_cast<AddressBarButton*>(sender());
	AddressBarButton *nextButton = qobject_cast<AddressBarButton*>(hBoxLayout->itemAt(button->index() + 1)->widget());

	QDir d(button->currentPath());
	foreach (QFileInfo folder, d.entryInfoList(QStringList(), QDir::NoDotAndDotDot | QDir::AllDirs | QDir::NoSymLinks)) {

		QAction *action = new QAction(QFileIconProvider().icon(folder), folder.fileName(), menu);
		// Check if the new submenu has one of its items already displayed, then make it bold
		if (nextButton != NULL && action->text() == nextButton->text()) {
			QFont font = action->font();
			font.setBold(true);
			action->setFont(font);
		}
		menu->addAction(action);
		button->addAction(action);
	}

	// Then display the menu
	QPoint p(button->geometry().right() - 36, button->geometry().y() + button->geometry().height() - 1);
	menu->popup(mapToGlobal(p), 0);
}
