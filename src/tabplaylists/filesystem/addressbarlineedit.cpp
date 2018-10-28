#include "addressbarlineedit.h"

#include "addressbar.h"

#include <QApplication>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPainter>

#include <QtDebug>

AddressBarLineEdit::AddressBarLineEdit(AddressBar *parent)
	: QLineEdit(parent)
	, _addressBar(parent)
	, _directoryList(nullptr)
{
	this->setMinimumHeight(parent->height());
	this->setFocusPolicy(Qt::ClickFocus);
	this->setAttribute(Qt::WA_MacShowFocusRect, false);
	this->addAction(style()->standardIcon(QStyle::SP_ComputerIcon), LeadingPosition);
	QAction *openHistory = this->addAction(style()->standardIcon(QStyle::SP_ArrowDown), TrailingPosition);
	connect(openHistory, &QAction::triggered, this, [=]() {
		QKeyEvent *ke;
		if (QDir::separator() == '/') {
			ke = new QKeyEvent(QEvent::KeyPress, Qt::Key_Slash, Qt::NoModifier);
		} else {
			ke = new QKeyEvent(QEvent::KeyPress, Qt::Key_Backslash, Qt::NoModifier);
		}
		qApp->postEvent(this, ke);
	});
}

void AddressBarLineEdit::focusOutEvent(QFocusEvent *e)
{
	if (_directoryList && _directoryList->hasFocus()) {
		return;
	}
	if (e->reason() != Qt::ActiveWindowFocusReason) {
		if (_directoryList) {
			_directoryList->deleteLater();
		}
		emit aboutToReloadAddressBar(QString());
	}
}

void AddressBarLineEdit::keyPressEvent(QKeyEvent *e)
{
	switch (e->key()) {
	case Qt::Key_Escape:{
			if (_directoryList) {
				_directoryList->deleteLater();
			}
			emit aboutToReloadAddressBar(QString());
			return;
		}
		break;
	case Qt::Key_Enter:
	case Qt::Key_Return:{
			QFileInfo fileInfo(this->text());
			if (fileInfo.isDir()) {
				if (_directoryList) {
					_directoryList->deleteLater();
				}
				emit aboutToReloadAddressBar(fileInfo.absoluteFilePath());
				return;
			} else {
				QMessageBox::critical(this, tr("Error"), QString(tr("Miam-Player cannot find « %1 ». Please check the name and retry.")).arg(this->text()));
				return;
			}
		}
		break;
	case Qt::Key_Backspace:{
			// Detect when separator was eaten
			if (this->text().endsWith(QDir::separator())) {
				QLineEdit::keyPressEvent(e);
				if (_directoryList) {
					_directoryList->cdUp(this->text());
				}
			} else {
				if (_directoryList) {
					QLineEdit::keyPressEvent(e);
					_directoryList->filterItems(this->text());
					return;
				}
			}
		}
		break;
	case Qt::Key_Up:
	case Qt::Key_Down:
	case Qt::Key_PageUp:
	case Qt::Key_PageDown: {
			if (_directoryList) {
				QKeyEvent keyEvent(QEvent::KeyPress, e->key(), e->modifiers());
				if (QCoreApplication::sendEvent(_directoryList, &keyEvent)) {
					this->setText(_directoryList->currentItem()->text());
				}
			} else {
				_directoryList = new AddressBarDirectoryList(text(), parentWidget()->parentWidget());
				_directoryList->show();
				_directoryList->move(_addressBar->x() + _addressBar->height(), _addressBar->height() - 3);
				connect(_directoryList, &QListWidget::itemClicked, this, [=](const QListWidgetItem *item) {
					_directoryList->deleteLater();
					emit aboutToReloadAddressBar(item->text());
				});
			}
		}
		break;
	default:{
			if (e->key() == QDir::separator()) {
				if (QFileInfo::exists(this->text())) {
					QFileInfo fileInfo(this->text());
					if (fileInfo.isDir()) {
						if (_directoryList == nullptr) {
							_directoryList = new AddressBarDirectoryList(fileInfo.absoluteFilePath(), parentWidget()->parentWidget());
							_directoryList->show();
							_directoryList->move(_addressBar->x() + _addressBar->height(), _addressBar->height() - 3);
							connect(_directoryList, &QListWidget::itemClicked, this, [=](const QListWidgetItem *item) {
								_directoryList->deleteLater();
								emit aboutToReloadAddressBar(item->text());
							});
						} else {
							_directoryList->cd(fileInfo.absoluteFilePath());
						}
					}
				}
			} else {
				if (_directoryList) {
					QLineEdit::keyPressEvent(e);
					_directoryList->filterItems(this->text());
					return;
				}
			}
		}
	}
	QLineEdit::keyPressEvent(e);
}

void AddressBarLineEdit::paintEvent(QPaintEvent *e)
{
	QLineEdit::paintEvent(e);
	QPainter p(this);
	p.setPen(QApplication::palette().highlight().color());
	p.drawRect(this->rect().adjusted(0, 0, -1, -1));

}
