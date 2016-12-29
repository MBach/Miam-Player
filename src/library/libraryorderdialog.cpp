#include "libraryorderdialog.h"

#include <settingsprivate.h>
#include "librarytreeview.h"

#include <QtDebug>

LibraryOrderDialog::LibraryOrderDialog(QWidget *parent)
	: QDialog(parent, Qt::Popup)
{
	setupUi(this);
	this->setAttribute(Qt::WA_DeleteOnClose);

	auto settings = SettingsPrivate::instance();
	artistTreeWidget->expandAll();
	albumTreeWidget->expandAll();
	artistAlbumTreeWidget->expandAll();
	yearTreeWidget->expandAll();

	artistTreeWidget->setAttribute(Qt::WA_MacShowFocusRect, false);
	albumTreeWidget->setAttribute(Qt::WA_MacShowFocusRect, false);
	artistAlbumTreeWidget->setAttribute(Qt::WA_MacShowFocusRect, false);
	yearTreeWidget->setAttribute(Qt::WA_MacShowFocusRect, false);

	switch (settings->insertPolicy()) {
	case SettingsPrivate::IP_Albums:
		albumTreeWidget->selectAll();
		albumTreeWidget->setFocus();
		break;
	case SettingsPrivate::IP_ArtistsAlbums:
		artistAlbumTreeWidget->selectAll();
		artistAlbumTreeWidget->setFocus();
		break;
	case SettingsPrivate::IP_Years:
		yearTreeWidget->selectAll();
		yearTreeWidget->setFocus();
		break;
	case SettingsPrivate::IP_Artists:
	default:
		artistTreeWidget->selectAll();
		artistTreeWidget->setFocus();
		break;
	}

	connect(artistTreeWidget, &QTreeWidget::itemClicked, this, [=]() {
		settings->setInsertPolicy(SettingsPrivate::IP_Artists);
		emit aboutToChangeHierarchyOrder();
		this->close();
	});
	connect(albumTreeWidget, &QTreeWidget::itemClicked, this, [=]() {
		settings->setInsertPolicy(SettingsPrivate::IP_Albums);
		emit aboutToChangeHierarchyOrder();
		this->close();
	});
	connect(artistAlbumTreeWidget, &QTreeWidget::itemClicked, this, [=]() {
		settings->setInsertPolicy(SettingsPrivate::IP_ArtistsAlbums);
		emit aboutToChangeHierarchyOrder();
		this->close();
	});
	connect(yearTreeWidget, &QTreeWidget::itemClicked, this, [=]() {
		settings->setInsertPolicy(SettingsPrivate::IP_Years);
		emit aboutToChangeHierarchyOrder();
		this->close();
	});
}
