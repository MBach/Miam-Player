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
	inline virtual QListWidget * artists() const { return _artists; }

	/** Required interface from AbstractSearchDialog class. */
	inline virtual QListWidget * albums() const { return _albums; }

	/** Required interface from AbstractSearchDialog class. */
	inline virtual QListWidget * tracks() const { return _tracks; }

protected:
	/** Custom rendering. */
	void paintEvent(QPaintEvent *);

private:
	/// XXX: factorize code
	void animate(qreal startValue, qreal stopValue);

public slots:
	void clear();

	/** Process results sent back from various search engines (local, remote). */
	virtual void processResults(SearchMediaPlayerPlugin::Request type, QList<QListWidgetItem*> results);

private slots:
	/** Local search for matching expressions. */
	void search(const QString &text);

	/** Expand this dialog to all available space. */
	void searchMoreResults(const QString &link);
};

#endif // SEARCHDIALOG_H
