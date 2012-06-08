#ifndef TAGEDITORTABLEWIDGET_H
#define TAGEDITORTABLEWIDGET_H

#include <QTableWidget>

class TagEditorTableWidget : public QTableWidget
{
	Q_OBJECT
public:
	TagEditorTableWidget(QWidget *parent = 0);

public slots:
	void addItemFromLibrary(const QPersistentModelIndex &index);
};

#endif // TAGEDITORTABLEWIDGET_H
