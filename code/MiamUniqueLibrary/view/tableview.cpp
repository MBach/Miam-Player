#include "tableview.h"

#include <QGuiApplication>
#include <QHeaderView>
#include <QScrollBar>

#include "model/albumdao.h"
#include "model/trackdao.h"

#include "albumitem.h"
#include "artistitem.h"
#include "coveritem.h"

#include <QtDebug>

TableView::TableView(QWidget *parent)
	: QTableView(parent), _jumpToWidget(new JumpToWidget(this)), _model(new QStandardItemModel(this))
	/*, _mediaPlaylist(new MediaPlaylist(this))*/
{
	_model->setColumnCount(4);
	_model->setHorizontalHeaderLabels({"", tr("Track"), tr("Title"), tr("Duration")});
	_proxyModel = new TableFilterProxyModel(this);
	_proxyModel->setSourceModel(_model);
	this->setModel(_proxyModel);
	this->setSortingEnabled(true);

	connect(this, &QTableView::doubleClicked, this, [=](const QModelIndex &index) {
		int r = index.row();
		QString uri = index.model()->index(r, 1).data(Miam::DF_URI).toString();
		qDebug() << Q_FUNC_INFO << uri;
		if (!uri.isEmpty()) {
			MediaPlayer::instance()->changeTrack(QMediaContent(QUrl::fromLocalFile(uri.mid(7))));
			viewport()->update();
		}
	});
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
	Q_UNUSED(node)
	if (!isVisible()) {
		return;
	}

	static bool isNewAlbum = false;
	//static bool isNewArtist = false;
	static bool isNewTrack = false;
	//static int trackCount = 0;

	switch (node->type()){
	case Miam::IT_Artist: {
		isNewTrack = false;
		ArtistItem *artist = new ArtistItem(node);
		_model->invisibleRootItem()->appendRow(artist);
		setSpan(_model->rowCount() - 1, 0, 1, 4);
		break;
	}
	case Miam::IT_Album: {
		isNewAlbum = true;
		isNewTrack = false;
		AlbumDAO *album = static_cast<AlbumDAO*>(node);
		qDebug() << Q_FUNC_INFO << album->artist() << (album->parentNode() == nullptr);
		CoverItem *cover = new CoverItem;
		cover->setText("Cover (" + album->title() + ")");
		QStandardItem *albumYear = new AlbumItem;
		if (album->year().isEmpty()) {
			albumYear->setText(album->title());
		} else {
			if (album->year().isEmpty() || album->year() == "0") {
				albumYear->setText(album->title());
			} else {
				albumYear->setText(album->title() + " [" + album->year() + "]");
			}
		}
		_model->invisibleRootItem()->appendRow({cover, albumYear});
		setSpan(_model->rowCount() - 1, 1, 1, 3);
		break;
	}
	case Miam::IT_Track: {
		isNewTrack = true;
		TrackDAO *trackDao = static_cast<TrackDAO*>(node);
		QStandardItem *track = new QStandardItem(trackDao->trackNumber());
		track->setData(trackDao->uri(), Miam::DF_URI);
		QStandardItem *title = new QStandardItem(trackDao->title());
		QStandardItem *length = new QStandardItem(trackDao->length());

		_model->invisibleRootItem()->appendRow({nullptr, track, title, length});
		//_mediaPlaylist->addMedia(QMediaContent(trackDao->uri()));
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
	/// TODO
}

void TableView::reset()
{
	_model->removeRows(0, _model->rowCount());
	setViewportMargins(0, 100, 0, 0);
}
