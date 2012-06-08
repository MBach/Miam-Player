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

	TagConverter *tagConverter;

public:
	TagEditor(QWidget *parent = 0);

signals:
	void closeTagEditor(bool);

public slots:
	/** Delete all rows. */
	void clear();

	void beforeAddingItems();

	void afterAddingItems();

private slots:
	/** Close this Widget and tells its parent to switch views. */
	void close();

	void commitChanges();

	void rollbackChanges();

	/** Display tags in separate QComboBoxes. */
	void displayTags();

	void recordChanges(QTableWidgetItem *item);

	void toggleTagConverter(bool);
};

#endif // TAGEDITOR_H
