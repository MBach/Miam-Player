#ifndef TAGEDITORTABLEWIDGET_H
#define TAGEDITORTABLEWIDGET_H

#include <QFileInfo>
#include <QTableWidget>

#include <fileref.h>

class TagEditorTableWidget : public QTableWidget
{
	Q_OBJECT

private:
	QList<QFileInfo> files;
	QList<TagLib::FileRef> tracks;

public:
	TagEditorTableWidget(QWidget *parent = 0);

	void updateColumnData(int column, QString text);

	void resetTable();

private:
	void fillTable(const QFileInfo fileInfo, const TagLib::FileRef f);

public slots:
	void addItemFromLibrary(const QPersistentModelIndex &index);
};

#endif // TAGEDITORTABLEWIDGET_H
