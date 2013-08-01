#include "libraryitemalbum.h"

#include <QtDebug>

LibraryItemAlbum::LibraryItemAlbum(const QString &album)
	: LibraryItem(album)
{
}

void LibraryItemAlbum::setYear(int year)
{
	this->setData(year, Qt::UserRole + 5);
}

int LibraryItemAlbum::year() const
{
	if (data(Qt::UserRole + 5).isValid()) {
		return data(Qt::UserRole + 5).toInt();
	} else {
		return -1;
	}
}

QString LibraryItemAlbum::coverPath() const
{
	return data(FILEPATH).toString();
}

void LibraryItemAlbum::setCoverPath(const QString &cover)
{
	setData(cover, FILEPATH);
}
