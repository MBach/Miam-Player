#include "listview.h"

#include <model/sqldatabase.h>
#include <libraryfilterproxymodel.h>

#include <QtDebug>

ListView::ListView(QWidget *parent)
	: QListView(parent)
	, _model(new UniqueLibraryItemModel(this))
{
	this->setModel(_model->proxy());
}

void ListView::createConnectionsToDB()
{
	qDebug() << Q_FUNC_INFO;
	auto db = SqlDatabase::instance();
	db->disconnect();
	connect(db, &SqlDatabase::loaded, this, [=]() {
		_model->proxy()->sort(0);
		_model->proxy()->setDynamicSortFilter(true);
	});
	connect(db, &SqlDatabase::aboutToLoad, this, &ListView::reset);
	connect(db, &SqlDatabase::nodeExtracted, _model, &UniqueLibraryItemModel::insertNode);
	connect(db, &SqlDatabase::aboutToUpdateNode, _model, &UniqueLibraryItemModel::updateNode);
	//connect(db, &SqlDatabase::aboutToCleanView, _model, &UniqueLibraryItemModel::cleanDanglingNodes);
	db->load();
}
