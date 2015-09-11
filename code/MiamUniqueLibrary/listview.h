#ifndef LISTVIEW_H
#define LISTVIEW_H

#include <QListView>
#include "miamuniquelibrary_global.h"
#include <libraryitemmodel.h>

//test
#include <libraryfilterproxymodel.h>

class MIAMUNIQUELIBRARY_LIBRARY ListView : public QListView
{
	Q_OBJECT
private:
	LibraryItemModel *_libraryModel;

public:
	explicit ListView(QWidget *parent = 0);

	inline LibraryItemModel *model() const { return _libraryModel; }

	void createConnectionsToDB();
};

#endif // LISTVIEW_H
