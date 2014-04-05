#include "addressbar.h"

#include "settings.h"
#include <QApplication>
#include <QDirIterator>
#include <QFileIconProvider>
#include <QPainter>
#include <QResizeEvent>

#include <QtDebug>

AddressBar::AddressBar(QWidget *parent) :
	QWidget(parent)
{
	_lineEdit = new QLineEdit(this);
	_lineEdit->installEventFilter(this);
	_lineEdit->setFrame(false);
	_lineEdit->setVisible(false);
	_lineEdit->setContentsMargins(1, 1, 1, 1);

	hBoxLayout = new QHBoxLayout(this);
	hBoxLayout->setContentsMargins(0, 0, 0, 0);
	hBoxLayout->setSpacing(0);
	hBoxLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));
	this->setLayout(hBoxLayout);

	// Create a special button with a computer icon, and shows a menu where previous items are stacked
	this->createRoot();

	menu = new AddressBarMenu(this);
	this->setMouseTracking(true);
	//this->setMaximumWidth(40);
	qDebug() << sizeHint();
}

bool AddressBar::eventFilter(QObject *obj, QEvent *e)
{
	if (obj == _lineEdit && e->type() == QEvent::FocusOut) {
		this->createRoot();
		this->init(_lineEdit->text());
		_lineEdit->hide();
	}
	return QWidget::eventFilter(obj, e);
}

void AddressBar::findAndHighlightButton(const QPoint &p)
{
	foreach (AddressBarButton *b, findChildren<AddressBarButton*>()) {
		if (b->rect().contains(b->mapFromGlobal(p))) {
			if (!b->isHighlighted()) {
				b->setHighlighted(true);
			}
			menu->moveOrHide(b);
		} else {
			b->setHighlighted(false);
		}
	}
}

void AddressBar::mousePressEvent(QMouseEvent *)
{
	_lineEdit->setGeometry(this->contentsRect());
	QLayoutItem *item = hBoxLayout->itemAt(hBoxLayout->count() - 2);

	AddressBarButton *button = qobject_cast<AddressBarButton*>(item->widget());
	if (button) {
		_lineEdit->setText(button->path());
	}
	_lineEdit->selectAll();
	while (hBoxLayout->count() > 0) {
		if (QLayoutItem *layoutItem = hBoxLayout->takeAt(0)) {
			if (layoutItem->widget()) {
				delete layoutItem->widget();
			} else if (layoutItem->spacerItem()) {
				delete layoutItem->spacerItem();
			} else if (layoutItem->layout()) {
				delete layoutItem->layout();
			}
		}
	}

	hBoxLayout->insertWidget(0, _lineEdit);
	_lineEdit->setMinimumHeight(this->rect().height());
	_lineEdit->show();
	_lineEdit->setFocus();
	qDebug() << hBoxLayout->count();
}

void AddressBar::paintEvent(QPaintEvent *)
{
	QPainter p(this);

	// Light gradient
	QPalette palette = QApplication::palette();
	QLinearGradient g(rect().topLeft(), rect().bottomLeft());
	if (Settings::getInstance()->isCustomColors()) {
		g.setColorAt(0, palette.base().color().lighter(110));
		g.setColorAt(1, palette.base().color());
	} else {
		g.setColorAt(0, palette.base().color());
		g.setColorAt(1, palette.window().color());
	}
	p.fillRect(rect(), g);

	// Frame
	p.setPen(palette.mid().color());
	if (isLeftToRight()) {
		p.drawLine(rect().topRight(), rect().bottomRight());
		p.drawLine(0, 0, rect().center().x(), 0);
	} else {
		p.drawLine(rect().topLeft(), rect().bottomLeft());
		p.drawLine(rect().width() - 1, 0, rect().center().x() - 2, 0);
	}
}

void AddressBar::resizeEvent(QResizeEvent *event)
{
	qDebug() << Q_FUNC_INFO << event->oldSize().width() << event->size().width();
	static const int arrowRectWidth = 15;
	static const int margin = 5;

	// How much space do we have after resizing?
	auto avalaibleWidth = [this, &event] () -> int {
		int w = event->size().width();
		foreach (AddressBarButton *b, findChildren<AddressBarButton*>()) {
			w -= b->width();
		}
		//qDebug() << "avalaibleWidth" << w;
		return w;
	};

	// One is expanding the address bar
	if (event->oldSize().width() < event->size().width()) {
		// Unstack hidden folder and try to append new buttons
		if (!_hiddenFolders.isEmpty()) {

			QDir lastFolder = _hiddenFolders.top();
			// We have enought space to move the last hidden folder to screen
			int lastFolderWidth = margin + fontMetrics().width(lastFolder.dirName()) + margin + arrowRectWidth;
			if (avalaibleWidth() >= lastFolderWidth) {
				this->createSubDirButtons(_hiddenFolders.pop());
			}
		}
	} else { // One is reducing the address bar
		if (avalaibleWidth() < 7) {
			QLayoutItem *item = hBoxLayout->takeAt(1);
			AddressBarButton *button = qobject_cast<AddressBarButton*>(item->widget());
			//new QListWidgetItem(QFileIconProvider().icon(QFileInfo(button->currentPath())), button->text(), menu);
			//menu->insertItem();
			qDebug() << "hide first buttons" << button->currentPath();
			_hiddenFolders.push(QDir(button->currentPath()));
			delete button;
		}
	}
	QWidget::resizeEvent(event);
}

/** Create a special root arrow button.*/
void AddressBar::createRoot()
{
	AddressBarButton *buttonArrow = new AddressBarButton(QDir::separator(), -1, this);
	connect(buttonArrow, &AddressBarButton::aboutToShowMenu, this, &AddressBar::showDrivesAndPreviousFolders);
	hBoxLayout->insertWidget(0, buttonArrow);
}

/** Append a button to the address bar to navigate through the filesystem. */
int AddressBar::createSubDirButtons(const QDir &path)
{
	AddressBarButton *buttonDir = new AddressBarButton(path.absolutePath(), hBoxLayout->count() - 1, this);
	buttonDir->setIcon(QFileIconProvider().icon(QFileInfo(path.absolutePath())));
	connect(buttonDir, &AddressBarButton::cdTo, this, &AddressBar::init);
	connect(buttonDir, &AddressBarButton::aboutToShowMenu, this, &AddressBar::showSubDirMenu);

	// Insert after the root button
	hBoxLayout->insertWidget(1, buttonDir);

	return buttonDir->width();
}

/** Init with an absolute path. Also used as a callback to a view. */
void AddressBar::init(const QString &initPath)
{
	qDebug() << Q_FUNC_INFO;
	this->clear();
	QDir dir(initPath);
	QList<int> listDirWidth;
	static const int arrowRectWidth = 15;
	static const int margin = 5;

	// Calculates the future width of the address bar
	while (!dir.isRoot()) {
		if (dir.dirName().isEmpty()) {
			listDirWidth.append(40);
		} else {
			bool hasSubDir = false;
			QDirIterator it(dir.absolutePath(), QDir::Dirs | QDir::NoDotAndDotDot);
			while (it.hasNext()) {
				it.next();
				if (it.fileInfo().isDir()) {
					hasSubDir = true;
					break;
				}
			}
			if (hasSubDir) {
				listDirWidth.append(margin + fontMetrics().width(dir.dirName()) + margin + arrowRectWidth);
			} else {
				listDirWidth.append(margin + fontMetrics().width(dir.dirName()));
			}
		}
		dir.cdUp();
	}
	listDirWidth.append(40);
	int totalWidth = 40;
	for (int i = 0; i < listDirWidth.count(); i++) {
		totalWidth += listDirWidth.at(i);
	}

	// Check if we need to create buttons or not
	dir.setPath(initPath);
	int availableWidthForNewButtons = totalWidth;
	while (!dir.isRoot()) {
		if (availableWidthForNewButtons > 0) {
			// Just append a new button to the address bar
			availableWidthForNewButtons -= this->createSubDirButtons(dir);
		} else {
			// Insert last item (or truncate if too large) first, and concatenate previous folders
			this->appendDirToRootButton(dir);
			//availableWidthForNewButtons +=
		}
		dir.cdUp();
	}
	if (availableWidthForNewButtons > 0) {
		this->createSubDirButtons(dir);
	} else {
		this->appendDirToRootButton(dir);
	}
	emit pathChanged(initPath);
}

/** Delete subdirectories located after the arrow button. */
void AddressBar::clear()
{
	// If we have something to delete after the Root button (which is never deleted)
	if (hBoxLayout->count() > 1) {
		// Delete items from the end
		while (hBoxLayout->count() > 2) {
			QLayoutItem *item = hBoxLayout->takeAt(1);
			if (item->widget()) {
				delete item->widget();
			}
		}
	}
}

void AddressBar::appendDirToRootButton(const QDir &previousDir)
{
	_hiddenFolders.append(previousDir);
	if (!menu->hasSeparator()) {
		menu->insertSeparator();
	}
}

/** Show logical drives (on Windows) or root item (on Unix). Also, when the path is too long, first folders are sent to this submenu. */
void AddressBar::showDrivesAndPreviousFolders()
{
	// Delete existing entries
	menu->clear();

	for (int i = _hiddenFolders.count() - 1; i >= 0; i--) {
		QDir d = _hiddenFolders.at(i);
		new QListWidgetItem(QFileIconProvider().icon(QFileInfo(d.currentPath())), d.dirName(), menu);
	}
	/// TODO Separator

	AddressBarButton *firstButton = qobject_cast<AddressBarButton*>(hBoxLayout->itemAt(0)->widget());
	AddressBarButton *nextButton = qobject_cast<AddressBarButton*>(hBoxLayout->itemAt(1)->widget());

	foreach (QFileInfo drive, QDir::drives()) {
		QString driveName = QDir::toNativeSeparators(drive.absoluteFilePath());
		if (driveName.length() > 1) {
			driveName.remove(QDir::separator());
		}
		QListWidgetItem *item =  new QListWidgetItem(QFileIconProvider().icon(drive), driveName, menu);
		item->setData(Qt::UserRole, drive.absoluteFilePath());
		if (!QDir(driveName).isReadable()) {
			item->setFlags(Qt::NoItemFlags);
		}
		// Check if the new submenu has one of its items already displayed, then make it bold
		if (nextButton != NULL && item->text() == nextButton->text()) {
			QFont font = item->font();
			font.setBold(true);
			item->setFont(font);
		}
	}

	// Then display the menu and the possibly empty list of folders before the first visible folder
	menu->moveOrHide(firstButton);
}

/** Show a popup menu with the content of the selected directory. */
void AddressBar::showSubDirMenu()
{
	// Delete existing entries
	menu->clear();
	AddressBarButton *button = qobject_cast<AddressBarButton*>(sender());
	AddressBarButton *nextButton = qobject_cast<AddressBarButton*>(hBoxLayout->itemAt(button->index() + 1)->widget());

	QDirIterator it(button->currentPath(), QDir::NoDotAndDotDot | QDir::Dirs | QDir::NoSymLinks);
	while (it.hasNext()) {
		it.next();
		QIcon icon = QFileIconProvider().icon(it.fileInfo());
		QListWidgetItem *item =  new QListWidgetItem(icon, it.fileName(), menu);
		item->setData(Qt::UserRole, it.fileInfo().absoluteFilePath());
		// Check if the new submenu has one of its items already displayed, then make it bold
		if (nextButton != NULL && item->text() == nextButton->text()) {
			QFont font = item->font();
			font.setBold(true);
			item->setFont(font);
		}
	}
	menu->moveOrHide(button);
}
