#include "listview.h"

#include <model/sqldatabase.h>
#include <libraryfilterproxymodel.h>

#include <QtDebug>

ListView::ListView(QWidget *parent)
	: QListView(parent)
	, _libraryModel(new LibraryItemModel(this))
{
	this->setModel(_libraryModel->proxy());
}

void ListView::createConnectionsToDB()
{
	qDebug() << Q_FUNC_INFO;
	auto db = SqlDatabase::instance();
	db->disconnect();
	connect(db, &SqlDatabase::aboutToLoad, this, &ListView::reset);
	//connect(db, &SqlDatabase::loaded, this, [=]() {
	//	qDebug() << "SqlDatabase::loaded" << _libraryModel->rowCount();
	//});
	connect(db, &SqlDatabase::nodeExtracted, this, [=](GenericDAO *node){
		node->setParentNode(nullptr);
		_libraryModel->insertNode(node);
	});
	connect(db, &SqlDatabase::aboutToUpdateNode, _libraryModel, &LibraryItemModel::updateNode);
	connect(db, &SqlDatabase::aboutToCleanView, _libraryModel, &LibraryItemModel::cleanDanglingNodes);
	db->load();
}
