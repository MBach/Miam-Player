#include "libraryorderdialog.h"

#include <qstandarditemmodel.h>
#include "settings.h"

#include <QtDebug>

#include "librarytreeview.h"

LibraryOrderDialog::LibraryOrderDialog(QWidget *parent) :
	QDialog(parent, Qt::Popup)
{
	setupUi(this);

	// Artists \ Albums \ Tracks
	QStandardItemModel *artistModel = new QStandardItemModel(this);
	artistModel->setHorizontalHeaderItem(0, new QStandardItem(tr("Artists \\ Albums")));
	artistModel->setHeaderData(0, Qt::Horizontal, LibraryTreeView::Artist, Qt::UserRole + 1);
	QStandardItem *artist = new QStandardItem("Artist");
	artistModel->appendRow(artist);
	for (int i = 1; i <= 1; i++) {
		QStandardItem *album = new QStandardItem("Album");
		artist->appendRow(album);
		for (int j = 1; j <= 2; j++) {
			album->appendRow(new QStandardItem("0" + QString::number(j) + ". track #" + QString::number(j)));
		}
	}
	artistTreeView->setModel(artistModel);

	bool disabled = false;

	// Albums \ Tracks
	QStandardItemModel *albumModel = new QStandardItemModel(this);
	albumModel->setHorizontalHeaderItem(0, new QStandardItem(tr("Albums")));
	albumModel->setHeaderData(0, Qt::Horizontal, LibraryTreeView::Album, Qt::UserRole + 1);
	QStandardItem *album = new QStandardItem("Album");
	albumModel->appendRow(album);
	for (int i = 1; i <= 2; i++) {
		album->appendRow(new QStandardItem("0" + QString::number(i) + ". track #" + QString::number(i)));
	}
	albumTreeView->setModel(albumModel);
	albumTreeView->setDisabled(disabled);

	// Artists - Albums \ Tracks
	QStandardItemModel *artistAlbumModel = new QStandardItemModel(this);
	artistAlbumModel->setHorizontalHeaderItem(0, new QStandardItem(tr("Artists – Albums")));
	artistAlbumModel->setHeaderData(0, Qt::Horizontal, LibraryTreeView::ArtistAlbum, Qt::UserRole + 1);
	QStandardItem *artistAlbum_1 = new QStandardItem("Artist – Album");
	artistAlbumModel->appendRow(artistAlbum_1);
	for (int i = 1; i <= 2; i++) {
		artistAlbum_1->appendRow(new QStandardItem("0" + QString::number(i) + ". track #" + QString::number(i)));
	}
	artistAlbumTreeView->setModel(artistAlbumModel);
	artistAlbumTreeView->setDisabled(disabled);

	// Year \ Artist - Album \ Tracks
	QStandardItemModel *yearModel = new QStandardItemModel(this);
	yearModel->setHorizontalHeaderItem(0, new QStandardItem(tr("Years")));
	yearModel->setHeaderData(0, Qt::Horizontal, LibraryTreeView::Year, Qt::UserRole + 1);
	QStandardItem *year = new QStandardItem("2014");
	yearModel->appendRow(year);
	QStandardItem *artistAlbum_2 = new QStandardItem("Artist – Album");
	year->appendRow(artistAlbum_2);
	for (int j = 1; j <= 2; j++) {
		artistAlbum_2->appendRow(new QStandardItem("0" + QString::number(j) + ". track #" + QString::number(j)));
	}
	yearTreeView->setModel(yearModel);
	yearTreeView->setDisabled(disabled);

	Settings *settings = Settings::getInstance();
	foreach (QTreeView *treeView, findChildren<QTreeView*>()) {
		treeView->expandAll();
		connect(treeView, &QTreeView::clicked, [=]() {
			foreach (QTreeView *treeView_2, findChildren<QTreeView*>()) {
				if (treeView == treeView_2) {
					treeView_2->setStyleSheet("border: 1px solid #66A7E8; background-color: #D1E8FF;");
					treeView_2->header()->setStyleSheet("QHeaderView::section { margin-left: 3px; margin-top: 4px; margin-right: 3px; margin-bottom: 4px; border: 0px; background-color: #D1E8FF; }");
					treeView_2->clearSelection();
					int i = treeView_2->model()->headerData(0, Qt::Horizontal, Qt::UserRole + 1).toInt();
					LibraryTreeView::ItemType insertPolicy = (LibraryTreeView::ItemType) i;
					// Rebuild library only if the click was on another treeview
					if (insertPolicy != settings->value("insertPolicy").toInt()) {
						settings->setValue("insertPolicy", insertPolicy);
						emit aboutToRedrawLibrary();
					}
				} else {
					treeView_2->setStyleSheet("");
					treeView_2->header()->setStyleSheet("");
				}
			}
			this->close();
		});
	}

	QTreeView *initialTreeView;
	switch (settings->value("insertPolicy").toInt()) {
	case LibraryTreeView::Album:
		initialTreeView = albumTreeView;
		break;
	case LibraryTreeView::ArtistAlbum:
		initialTreeView = artistAlbumTreeView;
		break;
	case LibraryTreeView::Year:
		initialTreeView = yearTreeView;
		break;
	case LibraryTreeView::Artist:
	default:
		initialTreeView = artistTreeView;
		break;
	}
	initialTreeView->setStyleSheet("border: 1px solid #66A7E8; background-color: #D1E8FF;");
	initialTreeView->header()->setStyleSheet("QHeaderView::section { margin-left: 3px; margin-top: 4px; margin-right: 3px; margin-bottom: 4px; border: 0px; background-color: #D1E8FF; }");
}

//#include <QPropertyAnimation>

void LibraryOrderDialog::show()
{
	QDialog::show();
	/*qDebug() << "pick one to populate this popup!";
	QPropertyAnimation *animation = new QPropertyAnimation(this, "pos");
	animation->setTargetObject(albumTreeView);
	QPoint topLeft = artistTreeView->rect().topLeft();
	QPoint topLeftEnd = topLeft;
	topLeftEnd.setX(topLeft.x() + artistTreeView->rect().width());
	animation->setStartValue(QVariant(topLeft));
	animation->setEndValue(QVariant(topLeftEnd));
	animation->setDuration(100);
	animation->start();
	yearTreeView->raise();
	artistAlbumTreeView->raise();
	albumTreeView->raise();
	artistTreeView->raise();*/
}
