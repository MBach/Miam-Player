#include "playlistheaderview.h"

#include "settings.h"

#include <QApplication>
#include <QStylePainter>

#include <QtDebug>

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
	this->setFrameShape(QFrame::NoFrame);

	// Context menu on header of columns
	columns = new QMenu(this);
	connect(columns, &QMenu::triggered, [=](const QAction *action) {
		int columnIndex = action->data().toInt();
		this->setSectionHidden(columnIndex, !this->isSectionHidden(columnIndex));
	});
}

/** Redefined for dynamic translation. */
void PlaylistHeaderView::changeEvent(QEvent *event)
{
	QHeaderView::changeEvent(event);
	if (model() && event->type() == QEvent::LanguageChange) {
		for (int i = 0; i < model()->columnCount(); i++) {
			model()->setHeaderData(i, Qt::Horizontal, tr(labels.at(i).toStdString().data()), Qt::DisplayRole);
		}
	}
}

void PlaylistHeaderView::setModel(QAbstractItemModel *model)
{
	QHeaderView::setModel(model);
	for (int i = 0; i < model->columnCount(); i++) {
		QString label = labels.at(i);
		model->setHeaderData(i, Qt::Horizontal, tr(label.toStdString().data()), Qt::DisplayRole);

		// Match actions with columns using index of labels
		QAction *actionColumn = new QAction(label, this);
		actionColumn->setData(i);
		actionColumn->setEnabled(actionColumn->text() != tr("Title"));
		actionColumn->setCheckable(true);
		actionColumn->setChecked(!isSectionHidden(i));

		// Then populate the context menu
		columns->addAction(actionColumn);
	}
}

void PlaylistHeaderView::contextMenuEvent(QContextMenuEvent *event)
{
	// Initialize values for the Header (label and horizontal resize mode)
	for (int i = 0; i < labels.size(); i++) {
		columns->actions().at(i)->setText(tr(labels.at(i).toStdString().data()));
	}

	for (int i = 0; i < this->count(); i++) {
		columns->actions().at(i)->setChecked(!this->isSectionHidden(i));
	}
	columns->exec(mapToGlobal(event->pos()));
}

void PlaylistHeaderView::paintSection(QPainter *, const QRect &rect, int logicalIndex) const
{
	QStylePainter p(this->viewport());
	QStyleOptionHeader opt;
	opt.initFrom(this);
	QLinearGradient vLinearGradient(rect.topLeft(), rect.bottomLeft());
	vLinearGradient.setColorAt(0, QApplication::palette().base().color().lighter(110));
	vLinearGradient.setColorAt(1,QApplication::palette().base().color());
	p.fillRect(rect, QBrush(vLinearGradient));
	p.drawText(rect, Qt::AlignCenter, model()->headerData(logicalIndex, Qt::Horizontal).toString());

	// Frame line
	p.setPen(QApplication::palette().mid().color());
	p.drawLine(rect.bottomLeft(), rect.bottomRight());
	if (isLeftToRight() && logicalIndex == 0) {
		p.drawLine(rect.topLeft(), rect.bottomLeft());
	} else if (!isLeftToRight() && logicalIndex == count() - 1){
		p.drawLine(rect.topLeft(), rect.bottomLeft());
	}
}

