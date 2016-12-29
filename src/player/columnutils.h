#ifndef COLUMNUTILS_H
#define COLUMNUTILS_H

#include <QStandardItemModel>
#include <QTableView>

/**
 * \brief		The ColumnUtils class
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class ColumnUtils
{
private:
	ColumnUtils();

public:
	static void resizeColumns(QTableView *table, const QList<int> &ratios);
};

#endif // COLUMNUTILS_H
