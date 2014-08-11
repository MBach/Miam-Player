#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include "abstractsearchdialog.h"
#include "sqldatabase.h"
#include "ui_searchdialog.h"

#include <QPropertyAnimation>

class SearchDialog : public AbstractSearchDialog, public Ui::SearchDialog
{
	Q_OBJECT
private:
	SqlDatabase _db;

	/** Used to make this dialog transparent to have a nice fading effect. */
	QPropertyAnimation *_animation;

	/** Duration of the fading effect. */
	QTimer *_timer;
	QRect _oldRect;

	QCheckBox *_checkBoxLibrary;

	QMap<QListView*, QList<QStandardItem*>> _hiddenItems;

public:
	/** Constructor. */
	explicit SearchDialog(const SqlDatabase &db, QWidget *parent = 0);

	/** Required interface from AbstractSearchDialog class. */
	virtual void addSource(QCheckBox *checkBox);

	/** String to look for on every registered search engines. */
	void setSearchExpression(const QString &text);

	/** Redefined from QWidget. */
	virtual void setVisible(bool visible);

	/** Required interface from AbstractSearchDialog class. */
	inline virtual QListView * artists() const { return _artists; }

	/** Required interface from AbstractSearchDialog class. */
	inline virtual QListView * albums() const { return _albums; }

	/** Required interface from AbstractSearchDialog class. */
	inline virtual QListView * tracks() const { return _tracks; }

protected:
	/** Custom rendering. */
	void paintEvent(QPaintEvent *);

private:
	/// XXX: factorize code
	void animate(qreal startValue, qreal stopValue);

public slots:
	void clear();

	/** Process results sent back from various search engines (local, remote). */
	virtual void processResults(Request type, const QStandardItemList &results);

private slots:
	void appendSelectedItem(const QModelIndex &index);

	/** Local search for matching expressions. */
	void search(const QString &text);

	/** Expand this dialog to all available space. */
	void searchMoreResults(const QString &link);
};

#endif // SEARCHDIALOG_H
