#include "addressbar.h"

#include <QDir>
#include <QFileIconProvider>
#include <QSpacerItem>

#include <QtDebug>

AddressBar::AddressBar(QWidget *parent) :
	QWidget(parent)
{
	hBoxLayout = new QHBoxLayout(this);
	hBoxLayout->setContentsMargins(0, 0, 0, 0);
	hBoxLayout->setSpacing(0);

	this->setLayout(hBoxLayout);
	this->setMinimumHeight(30);

	hBoxLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));
	this->createRoot();

	menu = new QMenu(this);
	connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(appendSubDir(QAction*)));
}

/** Create a special root arrow button.*/
void AddressBar::createRoot()
{
	AddressBarButton *buttonArrow = new AddressBarButton("/", -1, this);
	buttonArrow->setIcon(QIcon(":/icons/right-arrow"));
	buttonArrow->setIconSize(QSize(17, 7));
	connect(buttonArrow, SIGNAL(clicked()), this, SLOT(showDrives()));
	hBoxLayout->insertWidget(0, buttonArrow);
}

/** Append 2 buttons to the address bar to navigate through the filesystem. */
void AddressBar::createSubDirButtons(const QDir &path, bool insertFirst)
{
	AddressBarButton *buttonDir = new AddressBarButton(path.absolutePath(), hBoxLayout->count() - 1, this);
	buttonDir->setFlat(true);
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
	connect(buttonDir, SIGNAL(clicked()), this, SLOT(deleteFromNamedFolder()));

	if (insertFirst) {
		hBoxLayout->insertWidget(1, buttonDir);
	} else {
		hBoxLayout->insertWidget(hBoxLayout->count() - 1, buttonDir);
	}

	// Create an arrow only if there's at least one subfolder
	if (!path.entryInfoList(QStringList(), QDir::NoDotAndDotDot | QDir::AllDirs | QDir::NoSymLinks).isEmpty()) {
		AddressBarButton *buttonArrow = new AddressBarButton(path.absolutePath(), hBoxLayout->count() - 1, this);
		buttonArrow->setIcon(QIcon(":/icons/right-arrow"));
		buttonArrow->setIconSize(QSize(17, 7));
		buttonArrow->setFlat(true);
		connect(buttonArrow, SIGNAL(clicked()), this, SLOT(showSubDirMenu()));
		if (insertFirst) {
			hBoxLayout->insertWidget(2, buttonArrow);
		} else {
			hBoxLayout->insertWidget(hBoxLayout->count() - 1, buttonArrow);
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
		b->setIndex(i);
	}
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
			delete item->widget();
			delete item;
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
	AddressBarButton *addressBarButton = qobject_cast<AddressBarButton*>(sender());
	if (addressBarButton) {
		int after = addressBarButton->index() + 1;
		this->deleteFromArrowFolder(after);
	}
}

/** Show logical drives (on Windows) or root item (on Unix). */
void AddressBar::showDrives()
{
	// Delete existing entries
	menu->clear();

	AddressBarButton *firstButton = qobject_cast<AddressBarButton*>(hBoxLayout->itemAt(0)->widget());
	AddressBarButton *nextButton = qobject_cast<AddressBarButton*>(hBoxLayout->itemAt(1)->widget());

	foreach (QFileInfo drive, QDir::drives()) {
		QString driveName = drive.absoluteFilePath();
		if (driveName.length() > 1) {
			driveName.remove('/');
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

	// Then display the menu
	QPoint p(firstButton->geometry().x() - 20, firstButton->geometry().y() + firstButton->geometry().height() - 1);
	menu->exec(mapToGlobal(p));
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
	QPoint p(button->geometry().x() - 20, button->geometry().y() + button->geometry().height() - 1);
	menu->exec(mapToGlobal(p));
}
