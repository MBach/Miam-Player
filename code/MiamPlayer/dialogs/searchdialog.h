#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QDialog>
#include "ui_searchdialog.h"
#include "sqldatabase.h"

#include <QPropertyAnimation>

class SearchDialog : public QDialog, public Ui::SearchDialog
{
	Q_OBJECT
private:
	SqlDatabase _db;
	
	/** Used to make this dialog transparent to have a nice fading effect. */
	QPropertyAnimation *_animation;

	/** Duration of the fading effect. */
	QTimer *_timer;
	
public:
	explicit SearchDialog(const SqlDatabase &db, QWidget *parent = 0);
	
	virtual ~SearchDialog();
	
	void search(const QString &text);
	
	virtual void setVisible(bool visible);
	
protected:
	void paintEvent(QPaintEvent *);	
	
private:
	/// XXX: factorize code
	void animate(qreal startValue, qreal stopValue);	

public slots:
	void clear();
	
private slots:
	void searchMoreResults(const QString &link);
};

#endif // SEARCHDIALOG_H
