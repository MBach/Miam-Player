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
}

void AddressBarLineEdit::focusOutEvent(QFocusEvent *e)
{
	qDebug() << Q_FUNC_INFO << e;
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
			qDebug() << Q_FUNC_INFO << "enter pressed" << this->text();
			QFileInfo fileInfo(this->text());
			if (fileInfo.isDir()) {
				if (_directoryList) {
					_directoryList->deleteLater();
				}
				emit aboutToReloadAddressBar(fileInfo.absoluteFilePath());
				return;
			} else {
				qDebug() << Q_FUNC_INFO << "dir not recognized";
				QMessageBox::critical(this, tr("Error"), QString(tr("Miam-Player cannot find « %1 ». Please check the name and retry.")).arg(this->text()));
				return;
			}
		}
		break;
	case Qt::Key_Backspace:{
			// Detect when separator was eaten
			qDebug() << Q_FUNC_INFO << "About to process backspace on:" << this->text();
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
						qDebug() << "separator was detected" << fileInfo.absoluteFilePath();
						if (_directoryList == nullptr) {
							_directoryList = new AddressBarDirectoryList(fileInfo.absoluteFilePath(), parentWidget()->parentWidget());
							_directoryList->show();
							_directoryList->move(_addressBar->x() + _addressBar->height(), _addressBar->height() - 3);
							connect(_directoryList, &QListWidget::itemClicked, this, [=](const QListWidgetItem *item) {
								_directoryList->deleteLater();
								emit aboutToReloadAddressBar(item->text());
							});
						} else {
							qDebug() << "dir list already opened, updating" << fileInfo.absoluteFilePath();
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
