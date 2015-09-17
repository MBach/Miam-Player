#include "uniquelibrary.h"

#include "ui_uniquelibrary.h"

#include <QStandardItemModel>
#include <library/jumptowidget.h>
#include <filehelper.h>
#include <settingsprivate.h>
#include "uniquelibraryitemdelegate.h"

#include <QtDebug>

UniqueLibrary::UniqueLibrary(QWidget *parent) :
	QWidget(parent)
{
	setupUi(this);
	library->setItemDelegate(new UniqueLibraryItemDelegate(library->jumpToWidget(), library->model()->proxy()));
	library->setSelectionBehavior(QAbstractItemView::SelectRows);
	library->setSelectionMode(QAbstractItemView::ExtendedSelection);

	// Filter the library when user is typing some text to find artist, album or tracks
	//connect(searchBar, &QLineEdit::textEdited, library, &ListView::filterLibrary);
}
