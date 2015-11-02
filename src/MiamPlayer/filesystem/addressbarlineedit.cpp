#include "addressbarlineedit.h"

#include "addressbar.h"

#include <QKeyEvent>
#include <QMessageBox>

#include <QtDebug>

AddressBarLineEdit::AddressBarLineEdit(AddressBar *parent)
	: QLineEdit(parent)
	, _addressBar(parent)
	, _directoryList(nullptr)
{

}

void AddressBarLineEdit::focusOutEvent(QFocusEvent *e)
{
	qDebug() << Q_FUNC_INFO << e;
	if (_directoryList) {

	} else {
		emit aboutToReloadAddressBar(QString());
	}
}

void AddressBarLineEdit::keyPressEvent(QKeyEvent *e)
{
	switch (e->key()) {
	case Qt::Key_Enter:
	case Qt::Key_Return:{
			qDebug() << Q_FUNC_INFO << "enter pressed";
			QFileInfo fileInfo(this->text());
			if (fileInfo.isDir()) {
				if (_directoryList) {
					_directoryList->deleteLater();
				}
				emit aboutToReloadAddressBar(fileInfo.absoluteFilePath());
				return;
			} else {
				QMessageBox::critical(this, tr("Error"), QString(tr("Miam-Player cannot find « %1 ». Please check the name and retry.")).arg(this->text()));
			}
		}
		break;
	case Qt::Key_Backspace:{
			// Detect when separator was eaten
			qDebug() << Q_FUNC_INFO << this->text() << e->key();
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
			qDebug() << Q_FUNC_INFO << "key up or down pressed";
			if (_directoryList) {
				_directoryList->changeItemFromArrowKey(e->key());
				this->setText(_directoryList->currentItem()->text());
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
