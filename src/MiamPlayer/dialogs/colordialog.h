#ifndef COLORDIALOG_H
#define COLORDIALOG_H

#include <QColorDialog>

#include "reflector.h"

class CustomizeThemeDialog;

class ColorDialog : public QColorDialog
{
	Q_OBJECT
private:
	/** Pointer to the item containing all the instances of repaintables elements. */
	Reflector *reflector;

	CustomizeThemeDialog *_customizeThemeDialog;

public:
	ColorDialog(CustomizeThemeDialog *parent);

protected:
	void closeEvent(QCloseEvent *);
};

#endif // COLORDIALOG_H
