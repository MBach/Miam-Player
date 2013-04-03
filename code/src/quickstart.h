#ifndef QUICKSTARTTABLEWIDGET_H
#define QUICKSTARTTABLEWIDGET_H

#include <QWidget>

#include <QtDebug>

#include "ui_quickstart.h"

class QuickStart : public QWidget, public Ui::QuickStart
{
	Q_OBJECT
public:
	QuickStart(QWidget *parent = 0);

	/// Redefined
	/** The first time the player is launched, this function will scan for multimedia files. */
	void setVisible(bool b);

protected:
	void resizeEvent(QResizeEvent *);

private slots:
	void checkRow(int row, int);
};

#endif // QUICKSTARTTABLEWIDGET_H
