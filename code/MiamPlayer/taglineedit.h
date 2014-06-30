#ifndef TAGLINEEDIT_H
#define TAGLINEEDIT_H

#include "styling/lineedit.h"
#include "tagbutton.h"

class TagLineEdit : public LineEdit
{
	Q_OBJECT

	Q_PROPERTY(bool autoTransform READ isAutoTransform WRITE setAutoTransform)

private:
	bool autoTransform;

	QList<TagButton*> _tags;
	QTimer *_timerTag;

public:
	explicit TagLineEdit(QWidget *parent = 0);

	void addTag(const QString &tag);

	bool isAutoTransform() const;

	void setAutoTransform(bool enabled);

protected:
	virtual bool eventFilter(QObject *obj, QEvent *event);

	/** Redefined to display user input like closable "bubbles". */
	virtual void paintEvent(QPaintEvent *);

private:
	QStringList toStringList() const;

private slots:
	void createTag();

signals:
	void taglistHasChanged(const QStringList &tagslist);
};

#endif // TAGLINEEDIT_H
