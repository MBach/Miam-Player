#include "uniquelibrary.h"

#include "ui_uniquelibrary.h"

#include "filehelper.h"

#include <QDir>
#include <QPushButton>
#include <QSqlRecord>
#include <QSqlQuery>

#include <QtDebug>

#include "albumform.h"

UniqueLibrary::UniqueLibrary(QWidget *parent) :
	QWidget(parent), ui(new Ui::UniqueLibrary), _db(NULL)
{
	ui->setupUi(this);
	_model = new QStandardItemModel(this);
	_model->setColumnCount(4);

	_model->setHeaderData(0, Qt::Horizontal, tr("Cover"), Qt::DisplayRole);

	ui->library->setModel(_model);
}

void UniqueLibrary::setVisible(bool visible)
{
	qDebug() << Q_FUNC_INFO << visible;
	QWidget::setVisible(visible);
	if (visible) {
		_db->load();
	}
}

void UniqueLibrary::init(SqlDatabase *db)
{
	_db = db;

	// Build a tree directly by scanning the hard drive or from a previously saved file
	connect(_db, &SqlDatabase::aboutToLoad, this, &UniqueLibrary::reset);
	//connect(_db, &SqlDatabase::loaded, this, &UniqueLibrary::endPopulateTree);
	//connect(_db, &SqlDatabase::progressChanged, _circleProgressBar, &QProgressBar::setValue);
	connect(_db, &SqlDatabase::nodeExtracted, this, &UniqueLibrary::insertNode);
	connect(_db, &SqlDatabase::aboutToUpdateNode, this, &UniqueLibrary::updateNode);
}

void UniqueLibrary::insertNode(GenericDAO *node)
{
	if (!isVisible()) {
		return;
	}

	switch (node->type()){
	case GenericDAO::Artist: {
		QStandardItem *artist = new QStandardItem;
		artist->setText("[ " + node->title() + " ]");
		_model->appendRow(artist);
		break;
	}
	case GenericDAO::Album: {
		AlbumDAO *album = static_cast<AlbumDAO*>(node);

		QStandardItem *cover = new QStandardItem;
		QStandardItem *albumYear = new QStandardItem;
		if (album->year().isEmpty()) {
			albumYear->setText(album->title());
		} else {
			albumYear->setText(album->title() + " [" + album->year() + " ]");
		}
		_model->appendRow({cover, albumYear});
		break;
	}
	case GenericDAO::Track: {
		TrackDAO *trackDao = static_cast<TrackDAO*>(node);
		QStandardItem *track = new QStandardItem(trackDao->trackNumber());
		QStandardItem *title = new QStandardItem(trackDao->title());
		QStandardItem *length = new QStandardItem(trackDao->length());

		_model->appendRow({NULL, track, title, length});
		break;
	}
	}

}

void UniqueLibrary::updateNode(GenericDAO *)
{
	if (!isVisible()) {
		return;
	}
}

void UniqueLibrary::reset()
{
	_model->clear();
	_model->setHeaderData(0, Qt::Horizontal, tr("Cover"), Qt::DisplayRole);
}
