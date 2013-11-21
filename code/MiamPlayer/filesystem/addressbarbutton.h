#ifndef ADDRESSBARBUTTON_H
#define ADDRESSBARBUTTON_H

#include <QPushButton>

class AddressBarButton : public QPushButton
{
	Q_OBJECT
	Q_ENUMS(ButtonType)

public:
	enum ButtonType { Folder, Arrow };

private:
	ButtonType _type;

	QString path;

	int idx;

public:
	explicit AddressBarButton(ButtonType type, const QString &newPath, int index = -1, QWidget *parent = 0);

	inline ButtonType type() const { return _type; }

	QString currentPath() const;

	inline int index() const { return idx; }

	inline void setIndex(const int &index) { idx = index; }

//protected:
//	void mouseMoveEvent(QMouseEvent * e);
};

#endif // ADDRESSBARBUTTON_H
