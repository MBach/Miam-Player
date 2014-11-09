#ifndef CHANGEHIERARCHYBUTTON_H
#define CHANGEHIERARCHYBUTTON_H

#include <QPushButton>

class ChangeHierarchyButton : public QPushButton
{
	Q_OBJECT
public:
	explicit ChangeHierarchyButton(QWidget *parent = 0);

protected:
	virtual void paintEvent(QPaintEvent *);

	virtual void resizeEvent(QResizeEvent *event);
};

#endif // CHANGEHIERARCHYBUTTON_H
