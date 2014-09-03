#ifndef TAGLINEEDIT_H
#define TAGLINEEDIT_H

#include "styling/lineedit.h"
#include "tagbutton.h"

class TagLineEdit : public LineEdit
{
	Q_OBJECT

	Q_PROPERTY(bool _autoTransform READ isAutoTransform WRITE setAutoTransform)

private:
	bool _autoTransform;
	QList<TagButton*> _tags;
	QTimer *_timerTag;

public:
	explicit TagLineEdit(QWidget *parent = 0);

	void addTag(const QString &tag, int column = -1);

	/** Redefined to be able to move tag buttons. */
	void backspace();

	bool isAutoTransform() const;

	void setAutoTransform(bool enabled);

	inline QList<TagButton*> tags() const { return _tags; }

protected:
	virtual bool eventFilter(QObject *obj, QEvent *event);

	virtual void keyPressEvent(QKeyEvent *event);

	/** Redefined to automatically move cursor outside TagButton. */
	virtual void mousePressEvent(QMouseEvent *event);

	/** Redefined to display user input like closable "bubbles". */
	virtual void paintEvent(QPaintEvent *);

private:
	QStringList toStringList() const;

private slots:
	void createTag();

	void insertSpaces();

signals:
	void taglistHasChanged(const QStringList &tagslist);
};

#endif // TAGLINEEDIT_H
