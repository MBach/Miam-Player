#ifndef CHANGEHIERARCHYBUTTON_H
#define CHANGEHIERARCHYBUTTON_H

#include <QPushButton>
#include "miamlibrary_global.hpp"

/**
 * \brief		The ChangeHierarchyButton class is a button which changes how the LibraryTreeView class will display its content.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMLIBRARY_LIBRARY ChangeHierarchyButton : public QPushButton
{
	Q_OBJECT
public:
	explicit ChangeHierarchyButton(QWidget *parent = nullptr);

protected:
	virtual void paintEvent(QPaintEvent *) override;

	virtual void resizeEvent(QResizeEvent *event) override;
};

#endif // CHANGEHIERARCHYBUTTON_H
