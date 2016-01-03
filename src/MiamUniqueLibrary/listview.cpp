#include "listview.h"

#include <model/sqldatabase.h>
#include <libraryfilterproxymodel.h>
#include <libraryscrollbar.h>

#include <QGuiApplication>
#include <QPainter>
#include <QScrollBar>

#include <QtDebug>

ListView::ListView(QWidget *parent)
	: QListView(parent)
	, _model(new UniqueLibraryItemModel(this))
	, _jumpToWidget(new JumpToWidget(this))
{
	_model->proxy()->setDynamicSortFilter(false);
	this->setModel(_model->proxy());
	this->setVerticalScrollMode(ScrollPerPixel);
	LibraryScrollBar *vScrollBar = new LibraryScrollBar(this);
	this->setVerticalScrollBar(vScrollBar);
	connect(_jumpToWidget, &JumpToWidget::aboutToScrollTo, this, &ListView::jumpTo);
	connect(_model->proxy(), &MiamSortFilterProxyModel::aboutToHighlightLetters, _jumpToWidget, &JumpToWidget::highlightLetters);
	connect(vScrollBar, &QAbstractSlider::valueChanged, this, [=](int) {
		QModelIndex iTop = indexAt(viewport()->rect().topLeft());
		_jumpToWidget->setCurrentLetter(_model->currentLetter(iTop));
	});
	this->installEventFilter(this);
}

void ListView::createConnectionsToDB()
{
	auto db = SqlDatabase::instance();
	db->disconnect();
	//connect(db, &SqlDatabase::aboutToLoad, this, &ListView::reset);
	connect(db, &SqlDatabase::aboutToLoad, _model, [=]() {
		_model->removeRows(0, _model->rowCount());
	});
	connect(db, &SqlDatabase::tracksExtracted, _model, &UniqueLibraryItemModel::insertTracks);
	connect(db, &SqlDatabase::albumsExtracted, _model, &UniqueLibraryItemModel::insertAlbums);
	connect(db, &SqlDatabase::artistsExtracted, _model, &UniqueLibraryItemModel::insertArtists);
	connect(db, &SqlDatabase::aboutToUpdateNode, _model, &UniqueLibraryItemModel::updateNode);
	db->load(Settings::RSM_Flat);

	//connect(db, &SqlDatabase::nodeExtracted, _model, &UniqueLibraryItemModel::insertNode);
	//db->load(Settings::RSM_Hierarchical);
}

/** Redefined to override shortcuts that are mapped on simple keys. */
bool ListView::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::ShortcutOverride) {
		QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
		if (keyEvent) {
			// If one has assigned a simple key like 'N' to 'Skip Forward' we don't actually want to skip the track
			// IMHO, it's better to trigger the JumpTo widget to 'N' section
			if (65 <= keyEvent->key() && keyEvent->key() <= 90) {

				qDebug() << Q_FUNC_INFO;
				this->jumpTo(QString(keyEvent->key()));

				// We don't want this event to be propagated
				event->accept();
				return false;
			}
		}
		qDebug() << Q_FUNC_INFO << event << event->type();
	}
	return QListView::eventFilter(obj, event);
}

void ListView::paintEvent(QPaintEvent *event)
{
	int wVerticalScrollBar = 0;
	if (verticalScrollBar()->isVisible()) {
		wVerticalScrollBar = verticalScrollBar()->width();
	}
	if (QGuiApplication::isLeftToRight()) {
		///XXX: magic number
		_jumpToWidget->move(frameGeometry().right() - 22 - wVerticalScrollBar, 0);
	} else {
		_jumpToWidget->move(frameGeometry().left() + wVerticalScrollBar, 0);
	}

	if (_model->proxy()->rowCount() == 0) {
		QPainter p(this->viewport());
		p.drawText(this->viewport()->rect(), Qt::AlignCenter, tr("No matching results were found"));
	} else {
		QListView::paintEvent(event);
	}
}

void ListView::jumpTo(const QString &letter)
{
	QStandardItem *item = _model->letterItem(letter);
	if (item) {
		this->scrollTo(_model->proxy()->mapFromSource(item->index()), PositionAtTop);
	}
}
