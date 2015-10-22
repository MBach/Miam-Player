#ifndef CHANGEHIERARCHYBUTTON_H
#define CHANGEHIERARCHYBUTTON_H

#include <QPushButton>
#include "miamlibrary_global.hpp"

class MIAMLIBRARY_LIBRARY ChangeHierarchyButton : public QPushButton
{
	Q_OBJECT
public:
	explicit ChangeHierarchyButton(QWidget *parent = 0);

protected:
	virtual void paintEvent(QPaintEvent *);

	virtual void resizeEvent(QResizeEvent *event);
};

#endif // CHANGEHIERARCHYBUTTON_H
