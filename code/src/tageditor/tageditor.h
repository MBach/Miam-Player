#ifndef TAGEDITOR_H
#define TAGEDITOR_H

#include <QWidget>

#include "tagconverter.h"

#include "ui_tageditor.h"

using namespace Phonon;

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

protected:
	/** Redefined to filter context menu event for the cover album object. */
	bool eventFilter(QObject *obj, QEvent *event);

private:
	void replaceCover(const QVariant &cover);

public slots:
	/** Splits tracks into columns to be able to edit metadatas. */
	void addItemsToEditor(const QList<QPersistentModelIndex> &indexes);

	/** Clears all rows and comboboxes. */
	void clear();

	void removeCoverFromTag();

private slots:
	void applyCoverToAll(bool isAll);

	/** Closes this Widget and tells its parent to switch views. */
	void close();

	/** Saves all fields in the media. */
	void commitChanges();

	/** Displays tags in separate QComboBoxes. */
	void displayTags();

	/** Displays a cover only if the selected items have exactly the same cover. */
	void displayCover();

	void recordSingleItemChange(QTableWidgetItem *item);

	/** Cancels all changes made by the user. */
	void rollbackChanges();

	void toggleTagConverter(bool);

	void updateCells(QString text);

	void updateCover();

signals:
	void closeTagEditor(bool);

	void rebuildTreeView(QList<QPersistentModelIndex>);
};

#endif // TAGEDITOR_H
