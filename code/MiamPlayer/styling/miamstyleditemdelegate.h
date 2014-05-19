#ifndef MIAMSTYLEDITEMDELEGATE_H
#define MIAMSTYLEDITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QTableView>

class MiamStyledItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT
private:
	QTableView *_tableView;
	bool _fallback;

public:
	explicit MiamStyledItemDelegate(QTableView *parent, bool fallback);

	virtual void paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &index) const;
};

#endif // MIAMSTYLEDITEMDELEGATE_H
