#ifndef LISTVIEW_H
#define LISTVIEW_H

#include <QListView>
#include "miamuniquelibrary_global.h"
#include "uniquelibraryitemmodel.h"

class MIAMUNIQUELIBRARY_LIBRARY ListView : public QListView
{
	Q_OBJECT
private:
	UniqueLibraryItemModel *_model;

public:
	explicit ListView(QWidget *parent = 0);

	inline UniqueLibraryItemModel *model() const { return _model; }

	void createConnectionsToDB();
};

#endif // LISTVIEW_H
