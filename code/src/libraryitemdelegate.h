#ifndef LIBRARYITEMDELEGATE_H
#define LIBRARYITEMDELEGATE_H

#include <QStyledItemDelegate>

#include <QPainter>

#include "stareditor.h"

class LibraryItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT
private:
	QRect *titleRect;
	QRect *starsRect;

	StarEditor *starEditor;

	static int maxStars;

	QIcon favIcon;

	int _stars;

public:
	enum EditMode { Editable, ReadOnly };

	LibraryItemDelegate(QObject *parent = 0);
	~LibraryItemDelegate();

	QRect* title() const { return titleRect; }
	QRect* stars() const { return starsRect; }

	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;

	void setEditorData(QWidget *editor, const QModelIndex &index) const;
	void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

private slots:
	void commitAndCloseEditor(QWidget *);
	
};

#endif // LIBRARYITEMDELEGATE_H
