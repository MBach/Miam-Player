#include "tableview.h"

#include <QGuiApplication>
#include <QHeaderView>
#include <QScrollBar>

#include "model/albumdao.h"
#include "model/trackdao.h"

#include <QtDebug>

TableView::TableView(QWidget *parent)
	: QTableView(parent), _jumpToWidget(new JumpToWidget(this)), _model(new QStandardItemModel(this))
{
	_model->setColumnCount(4);
	_model->setHorizontalHeaderLabels({"", tr("Track"), tr("Title"), tr("Duration")});
	_proxyModel = new TableFilterProxyModel(this);
	_proxyModel->setSourceModel(_model);
	this->setModel(_proxyModel);
}

void TableView::setViewportMargins(int left, int top, int right, int bottom)
{
	qDebug() << Q_FUNC_INFO;
	QTableView::setViewportMargins(left, top, right, bottom);
}

void TableView::paintEvent(QPaintEvent *event)
{
	int wVerticalScrollBar = 0;
	if (verticalScrollBar()->isVisible()) {
		wVerticalScrollBar = verticalScrollBar()->width();
	}
	if (QGuiApplication::isLeftToRight()) {
		_jumpToWidget->move(frameGeometry().right() - 19 - wVerticalScrollBar, 0);
	} else {
		_jumpToWidget->move(frameGeometry().left() + wVerticalScrollBar, 0);
	}
	QTableView::paintEvent(event);
}

/** Reduces the size of the library when the user is typing text. */
void TableView::filterLibrary(const QString &filter)
{
	qDebug() << Q_FUNC_INFO << filter;
	if (filter.isEmpty()) {
		_proxyModel->setFilterRegExp(QRegExp());
		_proxyModel->sort(0, _proxyModel->sortOrder());
	} else {
		bool needToSortAgain = false;
		if (_proxyModel->filterRegExp().pattern().size() < filter.size() && filter.size() > 1) {
			needToSortAgain = true;
		}
		_proxyModel->setFilterRegExp(QRegExp(filter, Qt::CaseInsensitive, QRegExp::FixedString));
		if (needToSortAgain) {
			_proxyModel->sort(0, _proxyModel->sortOrder());
		}
	}
}

void TableView::insertNode(GenericDAO *node)
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
		_model->invisibleRootItem()->appendRow(artist);
		//_model->invisibleRootItem()->insertRow(i, artist);
		setSpan(i, 0, 1, 4);
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
		_model->invisibleRootItem()->appendRow({cover, albumYear});
		setSpan(i, 1, 1, 3);
		break;
	}
	case GenericDAO::Track: {
		isNewTrack = true;
		TrackDAO *trackDao = static_cast<TrackDAO*>(node);
		QStandardItem *track = new QStandardItem(trackDao->trackNumber());
		QStandardItem *title = new QStandardItem(trackDao->title());
		QStandardItem *length = new QStandardItem(trackDao->length());

		_model->invisibleRootItem()->appendRow({NULL, track, title, length});
		break;
	}
    default:
        break;
	}
}

void TableView::updateNode(GenericDAO *)
{
	if (!isVisible()) {
		return;
	}
}

void TableView::reset()
{
	_model->removeRows(0, _model->rowCount());
	setViewportMargins(0, 100, 0, 0);
}
