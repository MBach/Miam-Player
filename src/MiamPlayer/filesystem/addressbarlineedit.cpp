#include "addressbarlineedit.h"

#include "addressbar.h"

#include <QKeyEvent>

#include <QtDebug>

AddressBarLineEdit::AddressBarLineEdit(AddressBar *parent)
	: QLineEdit(parent)
	, _addressBar(parent)
{
	/*connect(this, &QLineEdit::editingFinished, this, [=]() {
		QFileInfo f(_lineEdit->text());
		if (f.isDir()) {
			_hBoxLayout->removeWidget(_lineEdit);
			delete _lineEdit;
			this->createRoot();
			this->init(f.absoluteFilePath());
		} else {
			QMessageBox::critical(this, tr("Error"), QString(tr("Miam-Player cannot find « %1 ». Please check the name and retry.")).arg(_lineEdit->text()));
			//_hBoxLayout->removeWidget(_lineEdit);
			//delete _lineEdit;
			//this->createRoot();
			qDebug() << Q_FUNC_INFO << f.absolutePath() << f.absoluteDir();
			//this->init(f.absoluteDir());
		}
	});*/
}

void AddressBarLineEdit::focusOutEvent(QFocusEvent *e)
{
	qDebug() << Q_FUNC_INFO << e;
	this->close();
	//_addressBar->createRoot();
	//_addressBar->init();
}

void AddressBarLineEdit::keyPressEvent(QKeyEvent *e)
{
	qDebug() << Q_FUNC_INFO << e->key();
	QLineEdit::keyPressEvent(e);
}
