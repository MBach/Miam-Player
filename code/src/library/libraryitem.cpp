#include "libraryitem.h"
#include "librarymodel.h"
#include "playlists/starrating.h"
#include "settings.h"

#include <QPainter>

#include <QtDebug>

LibraryItem::LibraryItem(const QString &text) :
	QStandardItem(text)
{
	setFont(Settings::getInstance()->font(Settings::LIBRARY));
}

LibraryItem::LibraryItem() :
	QStandardItem()
{
	this->setData(-1, SUFFIX);
}

void LibraryItem::setFilePath(const QString &filePath)
{
	setData(QVariant(filePath), Qt::UserRole+1);
}

void LibraryItem::setFilePath(int musicLocationIndex, const QString &fileName)
{
	setData(QVariant(musicLocationIndex), IDX_TO_ABS_PATH);
	setData(QVariant(fileName), REL_PATH_TO_MEDIA);
}

void LibraryItem::setDisplayedName(const char *name, int size)
{
	QByteArray byteArray(name, size);
	QVariant v(byteArray);
	setData(v.toString(), Qt::DisplayRole);
}
