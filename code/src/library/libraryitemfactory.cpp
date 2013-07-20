#include "libraryitemfactory.h"

LibraryItemFactory::LibraryItemFactory()
{
}

LibraryItem * LibraryItemFactory::createItem(int type)
{
	LibraryItem *libraryItem = NULL;
	switch(type) {
	case LibraryItem::Album:
		libraryItem = new LibraryItemAlbum();
		break;
	case LibraryItem::Artist:
		libraryItem = new LibraryItemArtist();
		break;
	//case LibraryItem::Letter:
	//	libraryItem = new LibraryItemLetter();
	//	break;
	case LibraryItem::Track:
		libraryItem = new LibraryItemTrack();
		break;
	}
	return libraryItem;
}
