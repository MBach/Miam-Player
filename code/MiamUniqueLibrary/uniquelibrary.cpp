#include "uniquelibrary.h"

#include "ui_uniquelibrary.h"

#include "filehelper.h"
#include "settingsprivate.h"

#include <QDir>
#include <QPushButton>
#include <QSqlRecord>
#include <QSqlQuery>

#include <QtDebug>

UniqueLibrary::UniqueLibrary(QWidget *parent) :
	QWidget(parent), ui(new Ui::UniqueLibrary)
{
	ui->setupUi(this);
	ui->library->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->library->setSelectionMode(QAbstractItemView::ExtendedSelection);
	ui->library->setShowGrid(false);

	// Filter the library when user is typing some text to find artist, album or tracks
	connect(ui->searchBar, &QLineEdit::textEdited, ui->library, &TableView::filterLibrary);
}

void UniqueLibrary::setVisible(bool visible)
{
	qDebug() << Q_FUNC_INFO << visible;
	QWidget::setVisible(visible);
	if (visible) {
		qDebug() << "hh" << ui->library->horizontalHeader();
		SqlDatabase::instance()->load();
		ui->library->horizontalHeader()->setHighlightSections(false);
		/*ui->library->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
		ui->library->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
		ui->library->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);*/
	}
}

void UniqueLibrary::init()
{
	qDebug() << Q_FUNC_INFO;
	auto db = SqlDatabase::instance();
	// Build a tree directly by scanning the hard drive or from a previously saved file
	connect(db, &SqlDatabase::aboutToLoad, ui->library, &TableView::reset);
	connect(db, &SqlDatabase::loaded, this, [=]() {
		ui->library->sortByColumn(0);
	});
	connect(db, &SqlDatabase::nodeExtracted, ui->library, &TableView::insertNode);
	connect(db, &SqlDatabase::aboutToUpdateNode, ui->library, &TableView::updateNode);
}
