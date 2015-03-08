#include "libraryorderdialog.h"

#include <qstandarditemmodel.h>
#include "settingsprivate.h"

#include "ui_libraryorderdialog.h"
#include "librarytreeview.h"

#include <QtDebug>

LibraryOrderDialog::LibraryOrderDialog(QWidget *parent) :
	QDialog(parent, Qt::Popup)
{
	setupUi(this);
}

void LibraryOrderDialog::setVisible(bool b)
{
	if (b) {
		QString track = tr("track");

		// Artists \ Albums \ Tracks
		QStandardItemModel *artistModel = new QStandardItemModel(this);
		artistModel->setHorizontalHeaderItem(0, new QStandardItem(tr("Artists \\ Albums")));
		artistModel->setHeaderData(0, Qt::Horizontal, SettingsPrivate::IP_Artists, Qt::UserRole);
		QStandardItem *artist = new QStandardItem(tr("Artist"));
		artistModel->appendRow(artist);
		for (int i = 1; i <= 1; i++) {
			QStandardItem *album = new QStandardItem(tr("Album"));
			artist->appendRow(album);
			for (int j = 1; j <= 2; j++) {
				album->appendRow(new QStandardItem("0" + QString::number(j) + ". " + track + " #" + QString::number(j)));
			}
		}
		auto m = artistTreeView->model();
		artistTreeView->setModel(artistModel);
		delete m;

		bool disabled = false;

		// Albums \ Tracks
		QStandardItemModel *albumModel = new QStandardItemModel(this);
		albumModel->setHorizontalHeaderItem(0, new QStandardItem(tr("Albums")));
		albumModel->setHeaderData(0, Qt::Horizontal, SettingsPrivate::IP_Albums, Qt::UserRole);
		QStandardItem *album = new QStandardItem(tr("Album"));
		albumModel->appendRow(album);
		for (int i = 1; i <= 2; i++) {
			album->appendRow(new QStandardItem("0" + QString::number(i) + ". " + track + " #" + QString::number(i)));
		}
		m = albumTreeView->model();
		albumTreeView->setModel(albumModel);
		delete m;
		albumTreeView->setDisabled(disabled);

		// Artists - Albums \ Tracks
		QStandardItemModel *artistAlbumModel = new QStandardItemModel(this);
		artistAlbumModel->setHorizontalHeaderItem(0, new QStandardItem(tr("Artists – Albums")));
		artistAlbumModel->setHeaderData(0, Qt::Horizontal, SettingsPrivate::IP_ArtistsAlbums, Qt::UserRole);
		QStandardItem *artistAlbum_1 = new QStandardItem(tr("Artist – Album"));
		artistAlbumModel->appendRow(artistAlbum_1);
		for (int i = 1; i <= 2; i++) {
			artistAlbum_1->appendRow(new QStandardItem("0" + QString::number(i) + ". " + track + " #" + QString::number(i)));
		}
		m = artistAlbumTreeView->model();
		artistAlbumTreeView->setModel(artistAlbumModel);
		delete m;
		artistAlbumTreeView->setDisabled(disabled);

		// Year \ Artist - Album \ Tracks
		QStandardItemModel *yearModel = new QStandardItemModel(this);
		yearModel->setHorizontalHeaderItem(0, new QStandardItem(tr("Years")));
		yearModel->setHeaderData(0, Qt::Horizontal, SettingsPrivate::IP_Years, Qt::UserRole);
		QStandardItem *year = new QStandardItem("2014");
		yearModel->appendRow(year);
		QStandardItem *artistAlbum_2 = new QStandardItem(tr("Artist – Album"));
		year->appendRow(artistAlbum_2);
		for (int j = 1; j <= 2; j++) {
			artistAlbum_2->appendRow(new QStandardItem("0" + QString::number(j) + ". " + track + " #" + QString::number(j)));
		}
		m = yearTreeView->model();
		yearTreeView->setModel(yearModel);
		delete m;
		yearTreeView->setDisabled(disabled);

		SettingsPrivate *settings = SettingsPrivate::instance();
		foreach (QTreeView *treeView, findChildren<QTreeView*>()) {
			treeView->expandAll();
			connect(treeView, &QTreeView::clicked, [=]() {
				foreach (QTreeView *treeView_2, findChildren<QTreeView*>()) {
					if (treeView == treeView_2) {
						treeView_2->clearSelection();
						int i = treeView_2->model()->headerData(0, Qt::Horizontal, Qt::UserRole).toInt();
						SettingsPrivate::InsertPolicy insertPolicy = (SettingsPrivate::InsertPolicy) i;
						// Rebuild library only if the click was on another treeview
						if (insertPolicy != settings->insertPolicy()) {
							settings->setInsertPolicy(insertPolicy);
							emit accept();
						}
					}
				}
				this->close();
			});
		}
	}
	QDialog::setVisible(b);
}

QString LibraryOrderDialog::headerValue() const
{
	switch (SettingsPrivate::instance()->insertPolicy()) {
	case SettingsPrivate::IP_Albums:
		return tr("Album");
	case SettingsPrivate::IP_ArtistsAlbums:
		return tr("Artist – Album");
	case SettingsPrivate::IP_Years:
		return tr("Year");
	case SettingsPrivate::IP_Artists:
	default:
		return tr("Artist \\ Album");
	}
}
