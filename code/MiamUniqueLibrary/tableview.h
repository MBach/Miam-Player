#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include <QStandardItemModel>
#include <QTableView>
#include "library/jumptowidget.h"
#include "model/tablefilterproxymodel.h"
#include "model/genericdao.h"

class TableView : public QTableView
{
	Q_OBJECT
private:
	JumpToWidget *_jumpToWidget;

	QStandardItemModel *_model;

	/** This view uses a proxy to specify how items in the Table should be ordered together. */
	TableFilterProxyModel *_proxyModel;

public:
	explicit TableView(QWidget *parent = 0);

	void setViewportMargins(int left, int top, int right, int bottom);

protected:
	virtual void paintEvent(QPaintEvent *event);

public slots:
	void insertNode(GenericDAO *node);
	void updateNode(GenericDAO *node);
	void filterLibrary(const QString &filter);
	void reset();
};

#endif // TABLEVIEW_H
