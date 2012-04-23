#ifndef SHORTCUTLINEEDIT_H
#define SHORTCUTLINEEDIT_H

#include <QLineEdit>

class ShortcutLineEdit : public QLineEdit
{
	Q_OBJECT
private:
	int typedKey;

public:
	ShortcutLineEdit(QWidget *parent = 0);

	inline int key() const { return typedKey; }

	QString setKey(int key);

protected:
	/** Redefined to enable special keys like Space. */
	void keyPressEvent(QKeyEvent *key);
	
private slots:
	/** Convert key pressed in the line edit. */
	void format(const QString &text);

};

#endif // SHORTCUTLINEEDIT_H
