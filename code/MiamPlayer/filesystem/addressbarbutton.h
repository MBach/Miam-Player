#ifndef ADDRESSBARBUTTON_H
#define ADDRESSBARBUTTON_H

#include <QDir>
#include <QPushButton>

class AddressBarButton : public QPushButton
{
	Q_OBJECT
private:
	QDir _path;

	QRect _textRect;
	QRect _arrowRect;
	bool _atLeastOneSubDir;
	bool _subMenuOpened;

public:
	explicit AddressBarButton(const QDir &newPath, QWidget *parent = 0);

	inline const QDir & path() const { return _path; }

	virtual QSize minimumSizeHint() const;

	void setHighlighted(bool b);

	inline bool isHighlighted() const { return _subMenuOpened; }

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

	void cdTo(const QDir &dir);
};

#endif // ADDRESSBARBUTTON_H
