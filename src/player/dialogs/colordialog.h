#ifndef COLORDIALOG_H
#define COLORDIALOG_H

#include <QColorDialog>

/// Forward declaration
class CustomizeThemeDialog;

/**
 * \brief		The ColorDialog class
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class ColorDialog : public QColorDialog
{
	Q_OBJECT
private:
	CustomizeThemeDialog *_customizeThemeDialog;

public:
	explicit ColorDialog(CustomizeThemeDialog *parent);
};

#endif // COLORDIALOG_H
