#ifndef TAGEDITOR_H
#define TAGEDITOR_H

#include <QWidget>

#include "tagconverter.h"

#include "ui_tageditor.h"

class TagEditor : public QWidget, public Ui::TagEditor
{
	Q_OBJECT

private:
	QMap<int, QComboBox*> combos;

	bool atLeastOneItemChanged;

	/// An automatic helper for writing tags following regExp
	TagConverter *tagConverter;

	static QStringList genres;

public:
	TagEditor(QWidget *parent = 0);

signals:
	void closeTagEditor(bool);

	void rebuildTreeView(QList<QPersistentModelIndex>);

public slots:
	/** Delete all rows. */
	void clear();

	void addItemsToEditor(const QModelIndexList &indexes);

private slots:
	/** Close this Widget and tells its parent to switch views. */
	void close();

	/** Save all fields in the media. */
	void commitChanges();

	/** Display tags in separate QComboBoxes. */
	void displayTags();

	void recordSingleItemChange(QTableWidgetItem *item);

	/** Cancel all changes made by the user. */
	void rollbackChanges();

	void toggleTagConverter(bool);

	void updateCells(QString text);
};

#endif // TAGEDITOR_H
