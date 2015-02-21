#include "uniquelibrary.h"

#include "ui_uniquelibrary.h"

#include "filehelper.h"

#include <QDir>
#include <QPushButton>
#include <QSqlRecord>
#include <QSqlQuery>

#include <QtDebug>

UniqueLibrary::UniqueLibrary(QWidget *parent) :
	QWidget(parent), ui(new Ui::UniqueLibrary), _db(NULL)
{
	ui->setupUi(this);
	_model = new QStandardItemModel(this);
	_model->setColumnCount(4);

	ui->library->setModel(_model);
	ui->library->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->library->setSelectionMode(QAbstractItemView::ExtendedSelection);
	//ui->library->setShowGrid(false);

	_model->setHorizontalHeaderLabels({"", tr("Track"), tr("Title"), tr("Duration")});
	ui->library->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->library->horizontalHeader()->setHighlightSections(false);
	ui->library->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
	ui->library->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
	ui->library->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
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
	qDebug() << Q_FUNC_INFO;
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

	static bool isNewAlbum = false;
	static bool isNewArtist = false;
	static bool isNewTrack = false;

	static int trackCount = 0;

	int i = _model->rowCount();
	switch (node->type()){
	case GenericDAO::Artist: {
		isNewTrack = false;
		QStandardItem *artist = new QStandardItem;
		artist->setText("[ " + node->title() + " ]");
		_model->appendRow(artist);
		ui->library->setSpan(i, 0, 1, 4);
		break;
	}
	case GenericDAO::Album: {
		isNewAlbum = true;
		isNewTrack = false;
		AlbumDAO *album = static_cast<AlbumDAO*>(node);
		QStandardItem *cover = new QStandardItem;
		QStandardItem *albumYear = new QStandardItem;
		if (album->year().isEmpty()) {
			albumYear->setText(album->title());
		} else {
			albumYear->setText(album->title() + " [" + album->year() + "]");
		}
		_model->appendRow({cover, albumYear});
		ui->library->setSpan(i, 1, 1, 3);
		break;
	}
	case GenericDAO::Track: {
		isNewTrack = true;
		TrackDAO *trackDao = static_cast<TrackDAO*>(node);
		QStandardItem *track = new QStandardItem(trackDao->trackNumber());
		QStandardItem *title = new QStandardItem(trackDao->title());
		QStandardItem *length = new QStandardItem(trackDao->length());

		_model->appendRow({NULL, track, title, length});
		break;
	}
	}
	//_set.insert(node);
}

void UniqueLibrary::updateNode(GenericDAO *)
{
	if (!isVisible()) {
		return;
	}
}

void UniqueLibrary::reset()
{
	_model->removeRows(0, _model->rowCount());
	//qDeleteAll(_set);
	ui->library->setViewportMargins(0, 100, 0, 0);

}
