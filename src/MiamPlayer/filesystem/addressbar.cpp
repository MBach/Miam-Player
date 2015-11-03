#include "addressbar.h"

#include "settingsprivate.h"
#include <QApplication>
#include <QDirIterator>
#include <QFileIconProvider>
#include <QPainter>
#include <QResizeEvent>
#include <QStandardPaths>

#include <QtDebug>

AddressBar::AddressBar(QWidget *parent)
	: QWidget(parent)
	, _hBoxLayout(new QHBoxLayout(this))
	, _menu(new AddressBarMenu(this))
	, _lastHighlightedButton(nullptr)
	, _lineEdit(nullptr)
	, _isDown(false)
{
	_hBoxLayout->setContentsMargins(0, 0, 0, 0);
	_hBoxLayout->setSpacing(0);
	this->setLayout(_hBoxLayout);

	// Create a special button with a computer icon, and shows a menu where previous items are stacked
	this->createRoot();

	this->setMouseTracking(true);
}

/// This is an exception in this source code to define a function depending if Q_OS_WIN exists or not
#ifdef Q_OS_WIN
#include <qt_windows.h>
// Retrieve volume names
QString AddressBar::getVolumeInfo(const QString &drive)
{
	WCHAR szVolumeName[256] ;
	WCHAR szFileSystemName[256];
	DWORD dwSerialNumber = 0;
	DWORD dwMaxFileNameLength = 256;
	DWORD dwFileSystemFlags = 0;
	bool ret = GetVolumeInformation((WCHAR *)drive.utf16(), szVolumeName, 256, &dwSerialNumber,
									&dwMaxFileNameLength, &dwFileSystemFlags, szFileSystemName, 256);
	if (!ret)
		return QString("");
	QString vName = QString::fromUtf16((const ushort *)szVolumeName);
	return vName.trimmed() + " (" + drive.left(drive.size() - 1) + ")";
}
#else
QString AddressBar::getVolumeInfo(const QString &drive)
{
	return drive;
}
#endif

/** Called by the popup menu when one is moving the mouse cursor. */
void AddressBar::findAndHighlightButton(const QPoint &p)
{
	for (AddressBarButton *b : findChildren<AddressBarButton*>()) {
		if (b->rect().contains(b->mapFromGlobal(p))) {
			if (!b->isHighlighted()) {
				b->setHighlighted(true);
				_lastHighlightedButton = b;
			}
			_menu->moveOrHide(b);
		} else {
			if (b != _lastHighlightedButton || _menu->isHidden()) {
				b->setHighlighted(false);
			}
		}
	}
}

void AddressBar::mousePressEvent(QMouseEvent *)
{
	this->feedLineEdit();
}

void AddressBar::paintEvent(QPaintEvent *)
{
	QPainter p(this);

	// Light gradient
	QPalette palette = QApplication::palette();
	QLinearGradient g(rect().topLeft(), rect().bottomLeft());
	g.setColorAt(0, palette.base().color());
	g.setColorAt(1, palette.window().color());
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
	p.drawLine(0, rect().height() - 1, rect().x() + rect().width(), rect().height() - 1);
}

void AddressBar::resizeEvent(QResizeEvent *event)
{
	//qDebug() << Q_FUNC_INFO << event->oldSize().width() << event->size().width();
	static const int arrowRectWidth = 15;
	static const int margin = 5;

	// How much space do we have after resizing for all buttons which are currently displayed?
	auto avalaibleWidth = [this, &event] () -> int {
		int w = event->size().width();
		for (AddressBarButton *b : this->findChildren<AddressBarButton*>()) {
			w -= b->width();
		}
		return w;
	};

	// One is expanding the address bar
	if (event->oldSize().width() < event->size().width()) {
		// Unstack hidden folder and try to append new buttons
		if (!_hiddenFolders.isEmpty()) {

			QDir lastFolder = _hiddenFolders.top();
			// Do we have enought space to move the last hidden folder to screen?
			int lastFolderWidth = margin + fontMetrics().width(lastFolder.dirName()) + margin + arrowRectWidth;
			if (avalaibleWidth() >= lastFolderWidth) {
				this->createSubDirButtons(_hiddenFolders.pop());
			}
		}
	} else { // One is reducing the address bar
		if (avalaibleWidth() < 7) {
			QLayoutItem *item = _hBoxLayout->itemAt(1);
			if (item && item->widget()) {
				AddressBarButton *button = qobject_cast<AddressBarButton*>(item->widget());

				// Root button, current button, and spacer item
				if (_hBoxLayout->count() == 3) {
					// Keep at least one button, and resize it to the minimum size
					if (button->width() > 70) {
						qDebug() << Q_FUNC_INFO << "we should reduce size" << button->minimumSizeHint();
						int actualTextWidth = fontMetrics().width(button->text());
						qDebug() << Q_FUNC_INFO << "text width" << actualTextWidth << button->text() << button->path();
						button->setText(fontMetrics().elidedText(button->text(), Qt::ElideRight, actualTextWidth - 5));
					}
				} else {
					_hiddenFolders.push(QDir(button->path()));
					delete button;
				}
			}
		}
	}
	QWidget::resizeEvent(event);
}

/** Delete subdirectories located after the arrow button. */
void AddressBar::clear()
{
	// If we have something to delete after the Root button (which is never deleted)
	if (_hBoxLayout->count() > 2) {
		// Delete items from the end
		while (_hBoxLayout->count() > 2) {
			QLayoutItem *item = _hBoxLayout->takeAt(1);
			if (item != nullptr && item->widget() != nullptr) {
				delete item->widget();
			}
		}
	}
	// Remove focus on root button
	AddressBarButton *root = qobject_cast<AddressBarButton*>(_hBoxLayout->itemAt(0)->widget());
	if (root->isHighlighted()) {
		root->setHighlighted(false);
	}

	// Delete hidden folders too
	if (!_hiddenFolders.isEmpty()) {
		_hiddenFolders.clear();
	}
}

/** Create a special root arrow button.*/
void AddressBar::createRoot()
{
	AddressBarButton *buttonArrow = new AddressBarButton(QDir("/"), this, true);
	connect(buttonArrow, &AddressBarButton::aboutToShowMenu, this, &AddressBar::showDrivesAndPreviousFolders);
	connect(buttonArrow, &AddressBarButton::triggerLineEdit, this, &AddressBar::feedLineEdit);
	_hBoxLayout->addWidget(buttonArrow);
	_hBoxLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));

}

/** Append a button to the address bar to navigate through the filesystem. */
int AddressBar::createSubDirButtons(const QDir &path)
{
	AddressBarButton *buttonDir = new AddressBarButton(path.absolutePath(), this);
	buttonDir->setIcon(QFileIconProvider().icon(QFileInfo(path.absolutePath())));
	connect(buttonDir, &AddressBarButton::aboutToShowMenu, this, [=]() {
		this->showSubDirMenu(buttonDir);
	});

	// Insert after the root button and before other ones.
	// Example: "C:\Music\Artist" will be created like this 1) 'Artist', 2) 'Music', 3) 'C:\'
	_hBoxLayout->insertWidget(1, buttonDir);
	return buttonDir->width();
}

/** Init with an absolute path. Also used as a callback to a view. */
void AddressBar::init(const QDir &initDir)
{
	static const int arrowRectWidth = 15;
	static const int margin = 5;

	_isDown = false;
	_currentDir = initDir;

	QDir dir(initDir);
	QDir dirTmp(initDir);
	this->clear();
	QList<int> listDirWidth;

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
				//qDebug() << "width" << dir.dirName();
				listDirWidth.append(margin + fontMetrics().width(dir.dirName()) + margin + arrowRectWidth);
			} else {
				listDirWidth.append(margin + fontMetrics().width(dir.dirName()));
			}
		}
		dir.cdUp();
	}
	listDirWidth.append(40);
	int simulatedWidth = 40;
	for (int i = 0; i < listDirWidth.count(); i++) {
		simulatedWidth += listDirWidth.at(i);
	}

	// Check if we need to create buttons or not
	dir = dirTmp;
	int availableWidthForNewButtons = width() - 40 - 40; // (root + drive)
	while (!dir.isRoot()) {
		if (availableWidthForNewButtons > 0) {
			// Just append a new button to the address bar
			availableWidthForNewButtons -= this->createSubDirButtons(dir);
		} else {
			// Insert last item (or truncate if too large) first, and concatenate previous folders
			_hiddenFolders.prepend(dir);
		}
		dir.cdUp();
	}
	if (availableWidthForNewButtons > 0) {
		this->createSubDirButtons(dir);
	} else {
		_hiddenFolders.prepend(dir);
	}
	emit aboutToChangePath(dirTmp);
}

void AddressBar::feedLineEdit()
{
	// Delete all items in the layout first
	QLayoutItem *item;
	while ((item = _hBoxLayout->takeAt(0))) {
		delete item->widget();
		delete item;
	}

	_lineEdit = new AddressBarLineEdit(this);
	_lineEdit->setGeometry(parentWidget()->rect());
	_lineEdit->setText(QDir::toNativeSeparators(_currentDir.absolutePath()));
	_hBoxLayout->addWidget(_lineEdit);
	_lineEdit->setFocus();
	connect(_lineEdit, &AddressBarLineEdit::aboutToReloadAddressBar, this, [=](const QString &dirPath) {
		_hBoxLayout->removeWidget(_lineEdit);
		delete _lineEdit;
		this->createRoot();
		if (dirPath.isEmpty()) {
			this->init(_currentDir);
		} else {
			this->init(dirPath);
		}
	});
}

/** Show logical drives (on Windows) or root item (on Unix). Also, when the path is too long, first folders are sent to this submenu. */
void AddressBar::showDrivesAndPreviousFolders()
{
	// Delete existing entries
	_menu->clear();

	for (int i = _hiddenFolders.count() - 1; i >= 0; i--) {
		QDir d = _hiddenFolders.at(i);
		QString text;
		if (d.dirName().isEmpty()) {
			text = getVolumeInfo(d.absolutePath());
		} else {
			text = d.dirName();
		}
		QListWidgetItem *item = new QListWidgetItem(QFileIconProvider().icon(QFileInfo(d.absolutePath())), text, _menu);
		item->setSizeHint(QSize(_menu->viewport()->width(), 24));
		item->setData(Qt::UserRole, d.absolutePath());
	}
	if (!_hiddenFolders.isEmpty()) {
		_menu->insertSeparator();
	}

	AddressBarButton *firstButton = qobject_cast<AddressBarButton*>(_hBoxLayout->itemAt(0)->widget());
	AddressBarButton *nextButton = qobject_cast<AddressBarButton*>(_hBoxLayout->itemAt(1)->widget());

	// Insert Desktop, Documents, Downloads, Pictures, Music, Videos
	QString desktop = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
	QString documents = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).first();
	QString downloads = QStandardPaths::standardLocations(QStandardPaths::DownloadLocation).first();
	QString pictures = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).first();
	QString music = QStandardPaths::standardLocations(QStandardPaths::MusicLocation).first();
	QString videos = QStandardPaths::standardLocations(QStandardPaths::MoviesLocation).first();

	QFileInfoList fil;
	fil << desktop << documents << downloads << pictures << music << videos;

	auto insertItemInMenu = [this, nextButton] (const QFileInfo &fileInfo, const QString &location) -> void {
		QListWidgetItem *item =  new QListWidgetItem(QFileIconProvider().icon(fileInfo), location, _menu);
		item->setSizeHint(QSize(_menu->viewport()->width(), 24));
		item->setData(Qt::UserRole, fileInfo.absoluteFilePath());
		if (!fileInfo.isReadable()) {
			item->setFlags(Qt::NoItemFlags);
		}
		// Check if the new submenu has one of its items already displayed, then make it bold
		if (nextButton != nullptr && fileInfo.absoluteFilePath() == nextButton->path().absolutePath()) {
			QFont font = item->font();
			font.setBold(true);
			item->setFont(font);
		}
	};

	// Insert standard locations
	for (QFileInfo fileInfo : fil) {
		insertItemInMenu(fileInfo, fileInfo.baseName());
	}

	// Insert drives
	for (QFileInfo drive : QDir::drives()) {
		QString driveName = QDir::toNativeSeparators(drive.absoluteFilePath());
		if (driveName.length() > 1) {
			QString d = getVolumeInfo(driveName);
			if (!d.isEmpty()) {
				driveName = d;
			}
		}
		insertItemInMenu(drive, driveName);
	}

	// Then display the menu and the possibly empty list of folders before the first visible folder
	_menu->moveOrHide(firstButton);
}

/** Show a popup menu with the content of the selected directory. */
void AddressBar::showSubDirMenu(AddressBarButton *button)
{
	// Delete existing entries
	_menu->clear();
	AddressBarButton *nextButton = nullptr;

	for (int i = 1; i < _hBoxLayout->count() - 2; i++) {
		QLayoutItem *layoutItem = _hBoxLayout->itemAt(i);
		if (layoutItem != nullptr && layoutItem->widget() != nullptr && _hBoxLayout->itemAt(i)->widget() == button) {
			nextButton = qobject_cast<AddressBarButton*>(_hBoxLayout->itemAt(i + 1)->widget());
			break;
		}
	}

	QDirIterator it(button->path().absolutePath(), QDir::NoDotAndDotDot | QDir::Dirs | QDir::NoSymLinks);
	while (it.hasNext()) {
		it.next();

		QIcon icon = QFileIconProvider().icon(it.fileInfo());
		QListWidgetItem *item =  new QListWidgetItem(icon, it.fileName(), _menu);
		QString absDirPath = it.fileInfo().absoluteFilePath();
		item->setSizeHint(QSize(_menu->viewport()->width(), 24));
		item->setData(Qt::UserRole, absDirPath);

		// Check if the new submenu has one of its items already displayed, then make it bold
		if (nextButton != nullptr && nextButton->path().absolutePath() == it.fileInfo().absoluteFilePath()) {
			QFont font = item->font();
			font.setBold(true);
			item->setFont(font);
		}
	}
	_menu->sortItems();
	_menu->moveOrHide(button);
}
