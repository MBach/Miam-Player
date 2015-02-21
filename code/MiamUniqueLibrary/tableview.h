#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include <QTableView>
#include "library/jumptowidget.h"

class TableView : public QTableView
{
	Q_OBJECT
private:
	JumpToWidget *_jumpToWidget;

public:
	explicit TableView(QWidget *parent = 0);

	void setViewportMargins(int left, int top, int right, int bottom);

protected:
	virtual void paintEvent(QPaintEvent *event);
};

#endif // TABLEVIEW_H
