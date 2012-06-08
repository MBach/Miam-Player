#ifndef COLORDIALOG_H
#define COLORDIALOG_H

#include <QColorDialog>

#include "reflector.h"

class ColorDialog : public QColorDialog
{
	Q_OBJECT
private:
	/** Pointer to the item containing all the instances of repaintables elements. */
	Reflector *reflector;

public:
	ColorDialog(QWidget *parent);

	void setPaintableElements(Reflector *reflector);

protected:
	void closeEvent(QCloseEvent *);
};

#endif // COLORDIALOG_H
