#include "libraryitemdiscnumber.h"

LibraryItemDiscNumber::LibraryItemDiscNumber(int discNumber)
	: LibraryItem(QString("CD %1").arg(discNumber))
{
	this->setData(discNumber, DISC_NUMBER);
}
