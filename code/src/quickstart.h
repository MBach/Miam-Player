#ifndef QUICKSTARTTABLEWIDGET_H
#define QUICKSTARTTABLEWIDGET_H

#include <QWidget>

#include <QtDebug>

#include "ui_quickstart.h"

class QuickStart : public QWidget, public Ui::QuickStart
{
	Q_OBJECT
private:
	static const QList<int> ratios;

public:
	explicit QuickStart(QWidget *parent = 0);

	virtual bool eventFilter(QObject *, QEvent *e);

	/// Redefined
	/** The first time the player is launched, this function will scan for multimedia files. */
	void setVisible(bool b);

private slots:
	void checkRow(int row, int);
};

#endif // QUICKSTARTTABLEWIDGET_H
