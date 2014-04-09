#ifndef ADDRESSBARBUTTON_H
#define ADDRESSBARBUTTON_H

#include <QDir>
#include <QPushButton>

/// Forward declaration
class AddressBar;

class AddressBarButton : public QPushButton
{
	Q_OBJECT
private:
	QDir _path;

	QRect _textRect;
	QRect _arrowRect;
	bool _atLeastOneSubDir;
	bool _highlighted;

	AddressBar *_addressBar;

public:
	explicit AddressBarButton(const QDir &newPath, AddressBar *parent);

	inline const QDir & path() const { return _path; }

	virtual QSize minimumSizeHint() const;

	void setHighlighted(bool b);

	inline bool isHighlighted() const { return _highlighted; }

	inline QRect arrowRect() const { return _arrowRect; }

protected:
	/** Redefined. */
	virtual void mouseMoveEvent(QMouseEvent *);

	/** Redefined. */
	virtual void mousePressEvent(QMouseEvent *);

	/** Redefined. */
	virtual void paintEvent(QPaintEvent *);

signals:
	void aboutToShowMenu();
};

#endif // ADDRESSBARBUTTON_H
