#include "playlistheaderview.h"

#include "settings.h"

#include <QtDebug>

#include <QCoreApplication>

QStringList PlaylistHeaderView::labels = QStringList() << "#"
													   << QT_TRANSLATE_NOOP("PlaylistHeaderView", "Title")
													   << QT_TRANSLATE_NOOP("PlaylistHeaderView", "Album")
													   << QT_TRANSLATE_NOOP("PlaylistHeaderView", "Length")
													   << QT_TRANSLATE_NOOP("PlaylistHeaderView", "Artist")
													   << QT_TRANSLATE_NOOP("PlaylistHeaderView", "Rating")
													   << QT_TRANSLATE_NOOP("PlaylistHeaderView", "Year");

PlaylistHeaderView::PlaylistHeaderView(QWidget *parent) :
	QHeaderView(Qt::Horizontal, parent)
{
	this->setHighlightSections(false);
	this->setSectionsMovable(true);
	this->setSectionResizeMode(QHeaderView::Interactive);
	this->setStretchLastSection(true);
	this->setStyleSheet(Settings::getInstance()->styleSheet(this));

	// Context menu on header of columns
	columns = new QMenu(this);
	connect(columns, &QMenu::triggered, [=](const QAction *action) {
		int columnIndex = action->data().toInt();
		this->setSectionHidden(columnIndex, !this->isSectionHidden(columnIndex));
	});
}

void PlaylistHeaderView::setModel(QAbstractItemModel *model)
{
	for (int i = 0; i < model->columnCount(); i++) {
		QString label = labels.at(i);

		// Match actions with columns using index of labels
		QAction *actionColumn = new QAction(label, this);
		actionColumn->setData(i);
		actionColumn->setEnabled(actionColumn->text() != tr("Title"));
		actionColumn->setCheckable(true);
		actionColumn->setChecked(!isSectionHidden(i));

		// Then populate the context menu
		columns->addAction(actionColumn);
	}
	QHeaderView::setModel(model);
}

void PlaylistHeaderView::contextMenuEvent(QContextMenuEvent *event)
{
	// Initialize values for the Header (label and horizontal resize mode)
	for (int i = 0; i < labels.size(); i++) {
		//_playlistModel->setHeaderData(i, Qt::Horizontal, labels.at(i), Qt::DisplayRole);
		columns->actions().at(i)->setText(tr(labels.at(i).toStdString().data()));
	}

	for (int i = 0; i < this->count(); i++) {
		columns->actions().at(i)->setChecked(!this->isSectionHidden(i));
	}
	columns->exec(mapToGlobal(event->pos()));
}
