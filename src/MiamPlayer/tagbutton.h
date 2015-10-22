#ifndef TAGBUTTON_H
#define TAGBUTTON_H

#include <QLabel>
#include <QToolButton>

class TagLineEdit;

/**
 * \brief		The TagButton class is a small closable button which can be inserted in a QLineEdit.
 * \details		This class is a convenient way to use tags inside a line edit in order to execute conversion. It's simpler to use
 *				this kind of buttons instead of custom scripts like %artist%.
 * \author		Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class TagButton : public QWidget
{
	Q_OBJECT
private:
	TagLineEdit *_tagLineEdit;
	QLabel *_label;
	QToolButton *_closeButton;

	/// Match size of widget with spaces in a QLineEdit (?)
	int _position;
	int _spaceCount;

	/** Useful for the TagEditor only. */
	int _column;

public:
	explicit TagButton(const QString &tag, TagLineEdit *parent);

	inline QToolButton * closeButton() const { return _closeButton; }

	inline const QString text() const { return _label->text().toLower(); }

	inline int position() const { return _position; }
	inline void setPosition(int position) { _position = position; }

	inline int spaceCount() const { return _spaceCount; }
	inline void setSpaceCount(int spaceCount) { _spaceCount = spaceCount; }

	inline int column() const { return _column; }
	inline void setColumn(int column) { _column = column; }

	//QSize sizeHint() const;

protected:
	virtual void paintEvent(QPaintEvent *);

	virtual void showEvent(QShowEvent *event);

signals:
	void shown();
};

/** Overloaded to be able to filter similar terms. */
inline bool operator==(const TagButton &t1, const TagButton &t2)
{
	return t1.text() == t2.text();
}

#endif // TAGBUTTON_H
