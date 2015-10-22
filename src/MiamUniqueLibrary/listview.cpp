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
}

void ListView::createConnectionsToDB()
{
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
