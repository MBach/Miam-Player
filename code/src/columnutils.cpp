#include "columnutils.h"

#include <QScrollBar>

ColumnUtils::ColumnUtils()
{
}

void ColumnUtils::resizeColumns(QTableView *table, const QList<int> &ratios)
{
	int visibleRatio = 0;
	int resizableArea = table->size().width() - 4;
	if (table->verticalScrollBar()->isVisible()) {
		resizableArea -= table->verticalScrollBar()->size().width();
	}

	// Resize fixed columns first, and then compute the remaining width
	for (int c = 0; c < table->model()->columnCount(); c++) {
		if (!table->isColumnHidden(c)) {
			int ratio = ratios.at(c);
			// Fixed column
			if (ratio == 0) {
				table->resizeColumnToContents(c);
				resizableArea -= table->columnWidth(c) - 1;
			}
			visibleRatio += ratio;
		}
	}
	for (int c = 0; c < table->model()->columnCount(); c++) {
		int ratio = ratios.at(c);
		// Resizable column
		if (ratio != 0) {
			int s = resizableArea * ratio / visibleRatio ;
			if (!table->isColumnHidden(c)) {
				table->setColumnWidth(c, s);
			}
		}
	}
}
