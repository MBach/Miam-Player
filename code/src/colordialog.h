#ifndef COLORDIALOG_H
#define COLORDIALOG_H

#include <QColorDialog>

class ColorDialog : public QColorDialog
{
	Q_OBJECT
private:
	QList<QWidget *> targets;

public:
	ColorDialog(QWidget *parent);

	void setTargets(QList<QWidget *> t);

protected:
	void closeEvent(QCloseEvent *);
};

#endif // COLORDIALOG_H
