#ifndef TAGEDITORTABLEWIDGET_H
#define TAGEDITORTABLEWIDGET_H

#include <QFileInfo>
#include <QTableWidget>

#include <fileref.h>

class TagEditorTableWidget : public QTableWidget
{
	Q_OBJECT

private:
	/// An absolute file path is mapped with an item in the library. It's used to detect changes in tags.
	QMap<QString, QPersistentModelIndex> indexes;

public:
	TagEditorTableWidget(QWidget *parent = 0);

	void init();

	enum DataUserRole { MODIFIED	= Qt::UserRole+1,
						KEY			= Qt::UserRole+2
					  };

	inline QPersistentModelIndex index(const QString &absFilePath) const { return indexes.value(absFilePath); }

	void updateColumnData(int column, QString text);

	void resetTable();

private:
	void fillTable(const QFileInfo fileInfo, const TagLib::FileRef f);

public slots:
	void addItemFromLibrary(const QPersistentModelIndex &index);

	/** Redefined. */
	void clear();
};

#endif // TAGEDITORTABLEWIDGET_H
