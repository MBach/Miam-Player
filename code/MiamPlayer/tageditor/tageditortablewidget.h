#ifndef TAGEDITORTABLEWIDGET_H
#define TAGEDITORTABLEWIDGET_H

#include <QFileInfo>
#include <QTableWidget>

#include <cover.h>

/**
 * \brief		The TagEditorTableWidget class is a table where one can select lines in order to edit multiple tags.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class TagEditorTableWidget : public QTableWidget
{
	Q_OBJECT
private:
	/** This map is used to detect changes in tags. */
	QMap<int, QString> _indexes;

public:
	enum Columns { COL_Filename		= 0,
				   COL_Path			= 1,
				   COL_Title		= 2,
				   COL_Artist		= 3,
				   COL_ArtistAlbum	= 4,
				   COL_Album		= 5,
				   COL_Track		= 6,
				   COL_Disc			= 7,
				   COL_Year			= 8,
				   COL_Genre		= 9,
				   COL_Comment		= 10 };

	TagEditorTableWidget(QWidget *parent = 0);

	/** It's not possible to initialize header in the constructor. The object has to be instantiated completely first. */
	void init();

	enum DataUserRole { MODIFIED	= Qt::UserRole + 1,
						KEY			= Qt::UserRole + 2 };

	//inline QPersistentModelIndex index(const QString &absFilePath) const { return _indexes.value(absFilePath); }

	void resetTable();

	void updateCellData(int row, int column, const QString &text);

	void updateColumnData(int column, const QString &text);

public slots:
	/** Add items to the table in order to edit them. */
	bool addItemsToEditor(const QStringList &tracks, QMap<int, Cover*> &covers);

	/** Redefined. */
	void clear();
};

#endif // TAGEDITORTABLEWIDGET_H
