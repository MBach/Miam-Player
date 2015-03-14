#ifndef TAGEDITOR_H
#define TAGEDITOR_H

#include <model/selectedtracksmodel.h>
#include <model/sqldatabase.h>
#include <cover.h>
#include "tagconverter.h"

#include "ui_tageditor.h"

#include <QUrl>
#include <QWidget>

/**
 * \brief		The TagEditor class is the main class for editing metadata inside this soft.
 * \details		This Widget displays selected tracks in table form. One can interact by selecting one or multiples files at the same time.
 *				Relevant information are automatically collected and displayed in comboboxes. It is possible to tag lots of files in very
 *				few mouse clicks. It is also possible to load / extract covers from files.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class TagEditor : public QWidget, public Ui::TagEditor, public SelectedTracksModel
{
	Q_OBJECT

private:
	QMap<int, QComboBox*> _combos;

	static QStringList genres;

	QMap<int, Cover*> covers;
	QMap<int, Cover*> unsavedCovers;

	SqlDatabase *_db;

public:
	/** An automatic helper for writing tags following regExp. */
	TagConverter *tagConverter;

	explicit TagEditor(QWidget *parent = 0);

	virtual void init(SqlDatabase *db);

	virtual QStringList selectedTracks();

	virtual void updateSelectedTracks();

protected:
	virtual void dragEnterEvent(QDragEnterEvent *event);

	virtual void dragMoveEvent(QDragMoveEvent *event);

	virtual void dropEvent(QDropEvent *event);

	/** Redefined to filter context menu event for the cover album object. */
	virtual bool eventFilter(QObject *obj, QEvent *event);

private:
	void clearCovers(QMap<int, Cover *> &coversToRemove);

public slots:
	/** Splits tracks into columns to be able to edit metadatas. */
	void addItemsToEditor(const QStringList &tracks);

	/** Wrapper for addItemsToEditor. */
	void addItemsToEditor(const QList<QUrl> &tracks);

	/** Wrapper for addItemsToEditor. */
	void addItemsToEditor(const QList<TrackDAO> &tracks);

	/** Clears all rows and comboboxes. */
	void clear();

private slots:
	void applyCoverToAll(bool isForAll, Cover *cover);

	/** Closes this Widget and tells its parent to switch views. */
	void close();

	/** Saves all fields in the media. */
	void commitChanges();

	/** Displays tags in separate QComboBoxes. */
	void displayTags();

	/** Displays a cover only if all the selected items have exactly the same cover. */
	void displayCover();

	void recordSingleItemChange(QTableWidgetItem *item);

	void replaceCover(Cover *newCover);

	/** Cancels all changes made by the user. */
	void rollbackChanges();

	void updateCells(QString text);

signals:
	void aboutToCloseTagEditor();
};

#endif // TAGEDITOR_H
