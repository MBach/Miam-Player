#ifndef ADDRESSBARBUTTON_H
#define ADDRESSBARBUTTON_H

#include <QDir>
#include <QPushButton>

/// Forward declaration
class AddressBar;

/**
 * \brief		The AddressBarButton class represents a part of a long path.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 * \see			AddressBarMenu
 */
class AddressBarButton : public QPushButton
{
	Q_OBJECT
private:
	QDir _path;

	QRect _textRect;
	QRect _arrowRect;
	bool _atLeastOneSubDir;
	bool _highlighted;
	bool _isAbsoluteRoot;

	AddressBar *_addressBar;

public:
	explicit AddressBarButton(const QDir &newPath, AddressBar *parent, bool isAbsoluteRoot = false);

	inline const QDir & path() const { return _path; }

	void setHighlighted(bool b);

	inline bool isHighlighted() const { return _highlighted; }

	inline QRect arrowRect() const { return _arrowRect; }

protected:
	/** Redefined. */
	virtual void mouseMoveEvent(QMouseEvent *) override;

	/** Redefined. */
	virtual void mousePressEvent(QMouseEvent *) override;

	/** Redefined. */
	virtual void paintEvent(QPaintEvent *) override;

signals:
	void aboutToShowMenu();

	void triggerLineEdit();
};

#endif // ADDRESSBARBUTTON_H
