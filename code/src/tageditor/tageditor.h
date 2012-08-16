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

	/// Tracks to be renamed or retagged (identified by their row)
	QMap<int, QPersistentModelIndex> tracks;

	/// The value is true when a filename or a tag was changed
	QMap<int, bool> filenames;
	QMap<int, bool> tags;

	bool atLeastOneItemChanged;

	/// An automatic helper for writing tags following regExp
	TagConverter *tagConverter;

public:
	TagEditor(QWidget *parent = 0);

signals:
	void closeTagEditor(bool);

	void comboBoxChanged(QComboBox*);

	void tracksRenamed();

public slots:
	/** Delete all rows. */
	void clear();

	void beforeAddingItems();

	void addItemFromLibrary(const QPersistentModelIndex &index);

	void afterAddingItems();

private slots:
	/** Close this Widget and tells its parent to switch views. */
	void close();

	void commitChanges();

	void rollbackChanges();

	void updateTable(QString text);

	/** Display tags in separate QComboBoxes. */
	void displayTags();

	void recordChanges(QTableWidgetItem *item);

	void toggleTagConverter(bool);
};

#endif // TAGEDITOR_H
