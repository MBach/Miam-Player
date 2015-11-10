#ifndef LISTVIEW_H
#define LISTVIEW_H

#include <QListView>
#include <library/jumptowidget.h>
#include "miamuniquelibrary_global.hpp"
#include "uniquelibraryitemmodel.h"

/**
 * \brief		The ListView class is used to display thousands of tracks in a single list.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMUNIQUELIBRARY_LIBRARY ListView : public QListView
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
	void paintEvent(QPaintEvent *event);

public slots:
	void jumpTo(const QString &letter);
};

#endif // LISTVIEW_H
