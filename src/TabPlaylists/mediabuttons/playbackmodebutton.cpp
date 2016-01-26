#include "playbackmodebutton.h"

#include "settingsprivate.h"

#include <QtDebug>

PlaybackModeButton::PlaybackModeButton(QWidget *parent)
	: MediaButton(parent)
{
	_menu.addAction(tr("Stop after current"));
}

void PlaybackModeButton::contextMenuEvent(QContextMenuEvent *)
{

}

void PlaybackModeButton::setIconFromTheme(const QString &theme)
{
	qDebug() << Q_FUNC_INFO << theme << this->objectName() ;

	// The objectName in the UI file MUST match the alias in the QRC file!
	QString iconFile = ":/player/" + theme.toLower() + "/" + this->objectName().remove("Button");
	QIcon icon(iconFile);
	if (!icon.isNull()) {
		QPushButton::setIcon(QIcon(iconFile));
	} else {
		qDebug() << Q_FUNC_INFO << objectName();
	}
}

