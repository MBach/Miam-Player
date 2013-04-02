#ifndef COLUMNUTILS_H
#define COLUMNUTILS_H

#include <QStandardItemModel>
#include <QTableView>

class ColumnUtils
{
private:
	ColumnUtils();

public:
	static void resizeColumns(QTableView *table, const QList<int> &ratios);
};

#endif // COLUMNUTILS_H
