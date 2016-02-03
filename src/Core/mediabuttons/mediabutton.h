#ifndef MEDIABUTTON_H
#define MEDIABUTTON_H

#include <QPushButton>

#include "miamcore_global.h"

/**
 * \brief		The MediaButton class is useful for buttons like "Play", "Stop", etc.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY MediaButton : public QPushButton
{
	Q_OBJECT
public:
	MediaButton(QWidget *parent = nullptr);

	virtual ~MediaButton();

protected:
	virtual void paintEvent(QPaintEvent *) override;

public slots:
	/** Load an icon from a chosen theme in options. */
	virtual void setIconFromTheme(const QString &);

	/** Change the size of icons from the options. */
	void setSize(int);
};

#endif // MEDIABUTTON_H
