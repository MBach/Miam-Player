#ifndef MEDIABUTTON_H
#define MEDIABUTTON_H

#include <QPushButton>

#include "miamcore_global.h"

/**
Class for buttons like "Play", "Stop", etc.
*/
class MIAMCORE_LIBRARY MediaButton : public QPushButton
{
	Q_OBJECT
public:
	MediaButton(QWidget *parent = 0);

	/** Redefined to load custom icons saved in settings. */
	void setIcon(const QIcon &, bool toggled = false);

	/** Redefined to set shortcuts from settings at startup. */
	void setObjectName(const QString &);

signals:
	void mediaButtonChanged();

public slots:
	/** Load an icon from a chosen theme in options. */
	void setIconFromTheme(const QString &);

	/** Change the size of icons from the options. */
	void setSize(const int &);
};

#endif // MEDIABUTTON_H
