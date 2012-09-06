#ifndef ADDRESSBARBUTTON_H
#define ADDRESSBARBUTTON_H

#include <QPushButton>

class AddressBarButton : public QPushButton
{
	Q_OBJECT
private:
	QString path;

public:
	explicit AddressBarButton(const QString &newPath, QWidget *parent = 0);

	QString currentPath() const { return path; }
};

#endif // ADDRESSBARBUTTON_H
