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
	hBoxLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));

	this->setLayout(hBoxLayout);
	this->setMinimumHeight(30);

	styleSheetDir = "QPushButton {";
	styleSheetDir += " border: 0; border-radius: 0px; ";
	styleSheetDir += " height: 20px; ";
	styleSheetDir += " margin-left: 0px; margin-right: 0px; ";
	styleSheetDir += " padding-left: 2px; padding-right: 2px; ";
	styleSheetDir += "}";

	styleSheetDir += "QPushButton:hover {";
	styleSheetDir += " border: 1px solid #3c7fb1;";
	styleSheetDir += " border-radius: 0px;";
	styleSheetDir += " subcontrol-position: right; ";
	styleSheetDir += " background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #eaf6fd, stop: 0.4 #d7effc, stop: 0.41 #bde6fd, stop: 1.0 #a6d9f4);";
	styleSheetDir += "}";

	styleSheetDir += "QPushButton::menu-indicator:hover {";
	styleSheetDir += " border-left: 1px solid #3c7fb1; ";
	styleSheetDir += "}";

	styleSheetArrow = "QPushButton { border: 0; border-radius: 0px; height: 20px; padding-left: 2px; padding-right: 2px; } ";
	styleSheetArrow += "QPushButton:hover {";
	styleSheetArrow += " min-width: 17px; max-width: 17px; width: 17px;";
	styleSheetArrow += " border: 1px solid #3c7fb1;";
	styleSheetArrow += " border-radius: 0px;";
	styleSheetArrow += " background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #eaf6fd, stop: 0.4 #d7effc, stop: 0.41 #bde6fd, stop: 1.0 #a6d9f4);";
	styleSheetArrow += "}";

	menu = new QMenu(this);
	connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(appendSubDir(QAction*)));
}

/** Init with an absolute path. */
void AddressBar::init(const QString &initPath)
{
	QDir dir(initPath);
	while (!dir.isRoot()) {
		this->createSubDirButtons(dir, true);
		dir.cdUp();
	}
	this->createSubDirButtons(dir, true);
}

/** Append 2 buttons to the address bar to navigate through the filesystem. */
void AddressBar::createSubDirButtons(const QDir &path, bool insertFirst)
{
	AddressBarButton *buttonDir = new AddressBarButton(path.absolutePath(), this);
	buttonDir->setStyleSheet(styleSheetDir);
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
	connect(buttonDir, SIGNAL(clicked()), this, SLOT(deleteSubDir()));

	// The horizontal spacer is always the last widget
	QSpacerItem *spacer = hBoxLayout->takeAt(hBoxLayout->count() - 1)->spacerItem();
	if (insertFirst) {
		hBoxLayout->insertWidget(0, buttonDir);
	} else {
		hBoxLayout->addWidget(buttonDir);
	}

	// Create an arrow only if there's at least one subfolder
	if (!path.entryInfoList(QStringList(), QDir::NoDotAndDotDot | QDir::AllDirs | QDir::NoSymLinks).isEmpty()) {
		AddressBarButton *buttonArrow = new AddressBarButton(path.absolutePath(), this);
		buttonArrow->setStyleSheet(styleSheetArrow);
		buttonArrow->setIcon(QIcon(":/icons/right-arrow"));
		buttonArrow->setIconSize(QSize(17, 7));
		connect(buttonArrow, SIGNAL(clicked()), this, SLOT(showSubDirMenu()));
		if (insertFirst) {
			hBoxLayout->insertWidget(1, buttonArrow);
		} else {
			hBoxLayout->addWidget(buttonArrow);
		}
	}
	// Moves this spacer to the end
	hBoxLayout->addSpacerItem(spacer);
}

void AddressBar::updateWithNewFolder(const QString &path)
{
	this->createSubDirButtons(QDir(path));
}

/** Change the selected path then create subdirectories. */
void AddressBar::appendSubDir(QAction *action)
{
	QList<QWidget*> widgets = action->associatedWidgets();
	if (!widgets.isEmpty()) {
		AddressBarButton *button = NULL;
		foreach (QWidget *w, widgets) {
			button = qobject_cast<AddressBarButton*>(w);
			if (button) {
				break;
			}
		}

		int idxButton = -1;
		for (int i = 1; i < hBoxLayout->count() - 1; i++) {
			if (hBoxLayout->itemAt(i)->widget() == button) {
				idxButton = i;
				break;
			}
		}

		QDir subDir(button->currentPath());
		subDir.cd(action->text());
		this->deleteSubDir(idxButton);
		this->createSubDirButtons(subDir);
		emit pathChanged(subDir.absolutePath());
	}
}

/** Delete subdirectories when one clicks in the middle of this address bar. */
void AddressBar::deleteSubDir(int after)
{
	int idxPushButton = -1;

	// If the origin of the click is a folder or the arrow button just after
	if (after != idxPushButton) {
		idxPushButton = after - 1;
	} else {
		for (int i = 0; i < hBoxLayout->count() - 1; i++) {
			if (hBoxLayout->itemAt(i)->widget() == sender()) {
				idxPushButton = i;
				break;
			}
		}
	}

	// If we have something to delete after
	if (hBoxLayout->count() - 2 > idxPushButton) {
		// Delete items from the end (excluding the spacer)
		for (int i = hBoxLayout->count() - 2; i > idxPushButton + 1; i--) {
			QLayoutItem *item = hBoxLayout->takeAt(i);
			delete item->widget();
			delete item;
		}

		AddressBarButton *addressBarButton = qobject_cast<AddressBarButton*>(hBoxLayout->itemAt(idxPushButton+1)->widget());
		emit pathChanged(addressBarButton->currentPath());
	}
}

/** Show a popup menu with the content of the selected directory. */
void AddressBar::showSubDirMenu()
{
	// Delete existing entries
	menu->clear();

	// Create new entries
	AddressBarButton *button = qobject_cast<AddressBarButton*>(sender());
	QDir d(button->currentPath());
	foreach (QFileInfo folder, d.entryInfoList(QStringList(), QDir::NoDotAndDotDot | QDir::AllDirs | QDir::NoSymLinks)) {
		QAction *action = new QAction(QFileIconProvider().icon(folder), folder.fileName(), menu);
		menu->addAction(action);
		button->addAction(action);
	}

	// Then display the menu
	QPoint p(button->geometry().x() - 20, button->geometry().y() + button->geometry().height() - 1);
	menu->exec(mapToGlobal(p));
}
