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
	QList<QPersistentModelIndex> indexes;

public:
	TagEditorTableWidget(QWidget *parent = 0);

	void init();

	enum DataUserRole { MODIFIED	= Qt::UserRole+1,
						KEY			= Qt::UserRole+2
					  };

	inline QList<QFileInfo> fileList() const { return files; }
	inline QList<TagLib::FileRef> trackList() const { return tracks; }
	inline QList<QPersistentModelIndex> indexList() const { return indexes; }

	void updateColumnData(int column, QString text);

	void resetTable();

private:
	void fillTable(const QFileInfo fileInfo, const TagLib::FileRef f);

public slots:
	void addItemFromLibrary(const QPersistentModelIndex &index);
};

#endif // TAGEDITORTABLEWIDGET_H
