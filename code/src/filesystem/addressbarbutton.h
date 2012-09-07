#ifndef ADDRESSBARBUTTON_H
#define ADDRESSBARBUTTON_H

#include <QPushButton>

class AddressBarButton : public QPushButton
{
	Q_OBJECT
private:
	QString path;

	int idx;

public:
	explicit AddressBarButton(const QString &newPath, int index = -1, QWidget *parent = 0);

	QString currentPath() const;

	inline int index() const { return idx; }

	inline void setIndex(const int &index) { idx = index; }
};

#endif // ADDRESSBARBUTTON_H
