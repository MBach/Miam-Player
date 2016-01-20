#ifndef TAGEDITORTABLEWIDGET_H
#define TAGEDITORTABLEWIDGET_H

#include <QFileInfo>
#include <QTableWidget>

#include <cover.h>
#include <filehelper.h>

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
	TagEditorTableWidget(QWidget *parent = nullptr);

	/** It's not possible to initialize header in the constructor. The object has to be instantiated completely first. */
	void init();

	enum DataUserRole { MODIFIED	= Qt::UserRole + 1,
						KEY			= Qt::UserRole + 2 };

	void resetTable();

	void updateCellData(int row, int column, const QString &text);

	void updateColumnData(int column, const QString &text);

public slots:
	/** Add items to the table in order to edit them. */
	bool addItemsToEditor(const QStringList &tracks, QMap<int, Cover *> &covers);

	/** Redefined. */
	void clear();
};

#endif // TAGEDITORTABLEWIDGET_H
