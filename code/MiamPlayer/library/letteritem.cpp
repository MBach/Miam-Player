#include "letteritem.h"

LetterItem::LetterItem(const QString &text) :
	QStandardItem(text)
{}

int LetterItem::type() const
{
	return LibraryTreeView::IT_Letter;
}
