#ifndef LIBRARYITEMFACTORY_H
#define LIBRARYITEMFACTORY_H

#include "libraryitemalbum.h"
#include "libraryitemartist.h"
#include "libraryitemletter.h"
#include "libraryitemtrack.h"

class LibraryItemFactory
{
private:
	LibraryItemFactory();

public:
	static LibraryItem * createItem(int type);
};

#endif // LIBRARYITEMFACTORY_H
