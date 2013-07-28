#ifndef TAGEDITORTABLEWIDGET_H
#define TAGEDITORTABLEWIDGET_H

#include <QFileInfo>
#include <QTableWidget>

#include <fileref.h>

#include "filehelper.h"

/**
 * @brief The TagEditorTableWidget class is a table where one can select lines in order to edit multiple tags.
 */
class TagEditorTableWidget : public QTableWidget
{
	Q_OBJECT
private:
	/** An absolute file path is mapped with an item in the library. It's used to detect changes in tags. */
	QMap<QString, QPersistentModelIndex> indexes;

public:
	//enum Columns { ALBUM_COL = 5,
	//			   COVER_COL = 11};
	enum Columns { ALBUM_COL = 5 };

	TagEditorTableWidget(QWidget *parent = 0);

	/** It's not possible to initialize header in the constructor. The object has to be instantiated completely first. */
	void init();

	enum DataUserRole { MODIFIED	= Qt::UserRole+1,
						KEY			= Qt::UserRole+2
					  };

	inline QPersistentModelIndex index(const QString &absFilePath) const { return indexes.value(absFilePath); }

	void updateColumnData(int column, const QString &text);

	void resetTable();

public slots:
	/** Add items to the table in order to edit them. */
	bool addItemsToEditor(const QStringList &tracks, QMap<int, Cover*> &covers);

	/** Redefined. */
	void clear();
};

#endif // TAGEDITORTABLEWIDGET_H
