#ifndef LISTVIEW_H
#define LISTVIEW_H

#include <QTableView>
#include <library/jumptowidget.h>
#include "miamuniquelibrary_global.hpp"
#include "uniquelibraryitemmodel.h"


class MIAMUNIQUELIBRARY_LIBRARY ListView : public QTableView
{
	Q_OBJECT
private:
	UniqueLibraryItemModel *_model;

	JumpToWidget *_jumpToWidget;

public:
	explicit ListView(QWidget *parent = nullptr);

	inline UniqueLibraryItemModel *model() const { return _model; }

	void createConnectionsToDB();

	inline JumpToWidget* jumpToWidget() const { return _jumpToWidget; }

protected:
	virtual bool eventFilter(QObject *obj, QEvent *event) override;

	virtual void paintEvent(QPaintEvent *event) override;

public slots:
	void jumpTo(const QString &letter);

private slots:
	void mergeGrid();
};

#endif // LISTVIEW_H
