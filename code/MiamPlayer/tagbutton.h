#ifndef TAGBUTTON_H
#define TAGBUTTON_H

#include <QLabel>
#include <QToolButton>

class TagButton : public QWidget
{
	Q_OBJECT
private:
	QLabel *_label;
	QToolButton *_closeButton;

public:
	explicit TagButton(const QString &tag, QWidget *parent = 0);

	inline QToolButton * closeButton() const { return _closeButton; }

	inline const QString text() const { return _label->text().toLower(); }

protected:
	virtual void paintEvent(QPaintEvent *);
};

/** Overloaded to be able to filter similar terms. */
inline bool operator==(const TagButton &t1, const TagButton &t2)
{
	return t1.text() == t2.text();
}

#endif // TAGBUTTON_H
