#ifndef STOPBUTTON_H
#define STOPBUTTON_H

#include "mediabutton.h"

#include <QMenu>

/**
 * \brief		The StopButton class is a custom class for the Stop button only
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY StopButton : public MediaButton
{
private:
	QMenu _menu;

public:
	explicit StopButton(QWidget *parent = nullptr);

protected:
	virtual void contextMenuEvent(QContextMenuEvent *) override;
};

#endif // STOPBUTTON_H
