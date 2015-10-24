#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include "abstractsearchdialog.h"
#include "ui_searchdialog.h"

#include <QPropertyAnimation>

/// Forward declaration
class MainWindow;

/**
 * \brief		The SearchDialog class is a popup dialog which opens when one is typing text.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class SearchDialog : public AbstractSearchDialog, public Ui::SearchDialog
{
	Q_OBJECT
private:
	MainWindow *_mainWindow;

	/** Used to make this dialog transparent to have a nice fading effect. */
	QPropertyAnimation *_animation;

	/** Duration of the fading effect. */
	QTimer *_timer;
	QRect _oldRect;

	QCheckBox *_checkBoxLibrary;

	QMap<QListView*, QList<QStandardItem*>> _hiddenItems;
	bool _isMaximized;

public:
	/** Constructor. */
	explicit SearchDialog(MainWindow *mainWindow);

	/** String to look for on every registered search engines. */
	void setSearchExpression(const QString &text);

	/** Required interface from AbstractSearchDialog class. */
	virtual void addSource(QCheckBox *checkBox) override;

	/** Required interface from AbstractSearchDialog class. */
	inline virtual QListView * artists() const override { return _artists; }

	/** Required interface from AbstractSearchDialog class. */
	inline virtual QListView * albums() const override { return _albums; }

	/** Required interface from AbstractSearchDialog class. */
	inline virtual QListView * tracks() const override { return _tracks; }

protected:
	virtual bool eventFilter(QObject *obj, QEvent *event) override;

	/** Custom rendering. */
	virtual void paintEvent(QPaintEvent *) override;

private:
	/// XXX: factorize code
	void animate(qreal startValue, qreal stopValue);

	/** Start search again more but fetch more results. */
	void searchMoreResults();

public slots:
	virtual void aboutToProcessRemoteTracks(const std::list<TrackDAO> &tracks) override;

	void clear();

	void moveSearchDialog(int = -1, int = -1);

	/** Process results sent back from various search engines (local, remote). */
	virtual void processResults(Request type, const QStandardItemList &results) override;

private slots:
	void artistWasDoubleClicked(const QModelIndex &artistIndex);
	void albumWasDoubleClicked(const QModelIndex &albumIndex);
	void trackWasDoubleClicked(const QModelIndex &track);

	void appendSelectedItem(const QModelIndex &index);

	/** Local search for matching expressions. */
	void localSearch(const QString &text);

	/** Expand this dialog to all available space. */
	void searchLabelWasClicked(const QString &link);

	void toggleItems(bool enabled);
};

#endif // SEARCHDIALOG_H
