#ifndef TAGEDITOR_H
#define TAGEDITOR_H

#include <QWidget>

#include "ui_tageditor.h"

class TagEditor : public QWidget, public Ui::TagEditor
{
	Q_OBJECT
public:
	TagEditor(QWidget *parent = 0);

public slots:
	/** Delete all rows. */
	void clear();

private slots:
	void close();

signals:
	void closeTagEditor(bool);
};

#endif // TAGEDITOR_H
