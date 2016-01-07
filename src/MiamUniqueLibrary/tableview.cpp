#include "tableview.h"

#include <model/sqldatabase.h>
#include <libraryfilterproxymodel.h>
#include <libraryscrollbar.h>
#include <settingsprivate.h>

#include <QGuiApplication>
#include <QHeaderView>
#include <QPainter>
#include <QScrollBar>
#include <QSqlQuery>
#include <QSqlRecord>

#include <QtDebug>

TableView::TableView(QWidget *parent)
	: QTableView(parent)
	, _model(new UniqueLibraryItemModel(this))
	, _jumpToWidget(new JumpToWidget(this))
{
	_model->proxy()->setDynamicSortFilter(false);
	this->setModel(_model->proxy());
	this->setVerticalScrollMode(ScrollPerPixel);
	LibraryScrollBar *vScrollBar = new LibraryScrollBar(this);
	this->setVerticalScrollBar(vScrollBar);
	connect(_jumpToWidget, &JumpToWidget::aboutToScrollTo, this, &TableView::jumpTo);
	connect(_model->proxy(), &UniqueLibraryFilterProxyModel::aboutToHighlightLetters, _jumpToWidget, &JumpToWidget::highlightLetters);
	connect(vScrollBar, &QAbstractSlider::valueChanged, this, [=](int) {
		QModelIndex iTop = indexAt(viewport()->rect().topLeft());
		_jumpToWidget->setCurrentLetter(_model->currentLetter(iTop.sibling(iTop.row(), 1)));
	});
	this->installEventFilter(this);
	horizontalHeader()->resizeSection(0, SettingsPrivate::instance()->coverSize());
}

void TableView::createConnectionsToDB()
{
	auto db = SqlDatabase::instance();
	db->disconnect();
	connect(db, &SqlDatabase::aboutToLoad, _model, [=]() {
		_model->removeRows(0, _model->rowCount());
	});
	connect(db, &SqlDatabase::tracksExtracted, _model, &UniqueLibraryItemModel::insertTracks);
	connect(db, &SqlDatabase::albumsExtracted, _model, &UniqueLibraryItemModel::insertAlbums);
	connect(db, &SqlDatabase::artistsExtracted, _model, &UniqueLibraryItemModel::insertArtists);
	connect(db, &SqlDatabase::aboutToUpdateNode, _model, &UniqueLibraryItemModel::updateNode);
	db->load(Settings::RSM_Flat);
}

/** Redefined to override shortcuts that are mapped on simple keys. */
bool TableView::eventFilter(QObject *obj, QEvent *event)
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
	}
	return QTableView::eventFilter(obj, event);
}

/** Redefined to keep displayed covers untouched. */
void TableView::mouseMoveEvent(QMouseEvent *event)
{
	// Don't allow click on first column which contains the cover (it can erase parts of painted image)
	if (columnAt(event->pos().x()) != 0) {
		QTableView::mouseMoveEvent(event);
	}
}

/** Redefined to keep displayed covers untouched. */
void TableView::mousePressEvent(QMouseEvent *event)
{
	// Don't allow click on first column which contains the cover (it can erase parts of painted image)
	if (columnAt(event->pos().x()) != 0) {
		QTableView::mousePressEvent(event);
	}
}

void TableView::paintEvent(QPaintEvent *event)
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

	if (_model->rowCount() == 0) {
		QPainter p(this->viewport());
		p.drawText(this->viewport()->rect(), Qt::AlignCenter, tr("No matching results were found"));
	} else {
		QTableView::paintEvent(event);
	}
}

void TableView::jumpTo(const QString &letter)
{
	SqlDatabase *db = SqlDatabase::instance();
	QSqlQuery firstArtist(*db);
	firstArtist.prepare("SELECT name FROM artists WHERE name LIKE ? ORDER BY name LIMIT 1");
	firstArtist.addBindValue(letter + "%");
	if (firstArtist.exec() && firstArtist.next()) {
		for (QStandardItem *i : _model->findItems(firstArtist.record().value(0).toString(), Qt::MatchExactly, 1)) {
			if (i->type() == Miam::IT_Artist) {
				this->scrollTo(_model->proxy()->mapFromSource(i->index()), PositionAtTop);
				break;
			}
		}
	}
}
