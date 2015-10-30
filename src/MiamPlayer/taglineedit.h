#ifndef TAGLINEEDIT_H
#define TAGLINEEDIT_H

#include "styling/lineedit.h"
#include "tagbutton.h"

/**
 * \brief		The TagLineEdit class
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class TagLineEdit : public LineEdit
{
	Q_OBJECT

protected:
	QList<TagButton*> _tags;

public:
	explicit TagLineEdit(QWidget *parent = 0);

	void addTag(const QString &tag, int column = -1);

	/** Redefined to be able to move tag buttons.
	 * Backspace method is not virtual in QLineEdit, therefore keyPressEvent must be intercepted and eaten. */
	void backspace();

	inline QList<TagButton*> tags() const { return _tags; }

protected:
	virtual void closeTagButton(TagButton *t);

	virtual bool eventFilter(QObject *obj, QEvent *event);

	/** Redefined to be able to move TagButton when typing. */
	virtual void keyPressEvent(QKeyEvent *event);

	/** Redefined to automatically move cursor outside TagButton. */
	virtual void mousePressEvent(QMouseEvent *event);

	/** Redefined to display user input like closable "bubbles". */
	virtual void paintEvent(QPaintEvent *);

	QStringList toStringList() const;

public slots:
	void clearTextAndTags(const QString &txt);

private slots:
	/** TagButton instances are converted with whitespaces in the LineEdit in order to move them. */
	void insertSpaces();
};

#endif // TAGLINEEDIT_H
