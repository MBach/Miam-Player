#ifndef TAGEDITOR_H
#define TAGEDITOR_H

#include <model/selectedtracksmodel.h>
#include <model/sqldatabase.h>
#include <cover.h>
#include <abstractview.h>

#include "tagconverter.h"
#include "ui_tageditor.h"

#include <QUrl>
#include <QWidget>

class AcoustId;

/**
 * \brief		The TagEditor class is the main class for editing metadata inside this soft.
 * \details		This Widget displays selected tracks in table form. One can interact by selecting one or multiples files at the same time.
 *				Relevant information are automatically collected and displayed in comboboxes. It is possible to tag lots of files in very
 *				few mouse clicks. It is also possible to load / extract covers from files.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMTAGEDITOR_LIBRARY TagEditor : public AbstractView, public Ui::TagEditor, public SelectedTracksModel
{
	Q_OBJECT
private:
	AcoustId* _acoustId;

	QMap<int, QComboBox*> _combos;

	static QStringList genres;

	QMap<int, Cover*> _covers;
	QMap<int, Cover*> _unsavedCovers;

	QMap<int, QSet<QString>> _cacheData;

public:
	/** An automatic helper for writing tags following regExp. */
	TagConverter *tagConverter;

	explicit TagEditor(QWidget *parent = nullptr);

	void addDirectory(const QDir &dir);

	virtual QList<QUrl> selectedTracks() override;

	inline virtual ViewType type() const override { return VT_BuiltIn; }

	virtual void updateSelectedTracks() override;

	virtual bool viewProperty(Settings::ViewProperty vp) const override;

protected:
	/** Redefined to be able to retransltate User Interface at runtime. */
	virtual void changeEvent(QEvent *event) override;

	/** Redefined to save geometry silently. */
	virtual void closeEvent(QCloseEvent *event) override;

	/** Redefined. */
	virtual void dragEnterEvent(QDragEnterEvent *event) override;

	/** Redefined. */
	virtual void dragMoveEvent(QDragMoveEvent *event) override;

	/** Accepts dropping events by opening a new window. */
	virtual void dropEvent(QDropEvent *event) override;

	/** Redefined to filter context menu event for the cover album object. */
	virtual bool eventFilter(QObject *obj, QEvent *event) override;

private:
	/** Save data in order to be able to rollback. */
	void buildCache();

	void clearCovers(QMap<int, Cover *> &coversToRemove);

	/** Splits tracks into columns to be able to edit metadatas. */
	void addTracks(const QStringList &tracks);

public slots:
	/** Wrapper for addItemsToEditor. */
	void addItemsToEditor(const QList<QUrl> &tracks);

	/** Clears all rows and comboboxes. */
	void clear();

	virtual void setViewProperty(Settings::ViewProperty vp, QVariant value) override;

private slots:
	void applyCoverToAll(bool isForAll, Cover *cover);

	void autoFetchTags();

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
