#include "shortcutlineedit.h"

#include <QKeyEvent>

#include <QtDebug>

ShortcutLineEdit::ShortcutLineEdit(QWidget *parent) :
    QLineEdit(parent)
{
	connect(this, SIGNAL(textEdited(QString)), this, SLOT(format(QString)));
}

/** Redefined to enable special keys like Space. */
void ShortcutLineEdit::keyPressEvent(QKeyEvent *keyEvent)
{
	QString shortcut;
	typedKey = keyEvent->key();
	switch(keyEvent->key()) {
	case Qt::Key_Backspace:
		shortcut = tr("Backspace");
		break;
	case Qt::Key_Return:
		shortcut = tr("Return");
		break;
	case Qt::Key_Space:
		shortcut = tr("Space");
		break;
	case Qt::Key_Left:
		shortcut = tr("Left");
		break;
	case Qt::Key_Right:
		shortcut = tr("Right");
		break;
	case Qt::Key_Up:
		shortcut = tr("Up");
		break;
	case Qt::Key_Down:
		shortcut = tr("Down");
		break;
	case Qt::Key_Home:
		shortcut = tr("Home");
		break;
	case Qt::Key_End:
		shortcut = tr("End");
		break;
	case Qt::Key_Delete:
		shortcut = tr("Delete");
		break;
	case Qt::Key_PageUp:
		shortcut = tr("Page Up");
		break;
	case Qt::Key_PageDown:
		shortcut = tr("Page Down");
		break;
	case Qt::Key_Insert:
		shortcut = tr("Insert");
		break;
	/// Todo other keys like F1, ... , F12 ?
	default:
		QLineEdit::keyPressEvent(keyEvent);
	}
	if (!shortcut.isEmpty()) {
		setText(shortcut);
	}
	emit editingFinished();
}

/** Convert key pressed in the line edit. */
void ShortcutLineEdit::format(const QString &s)
{
	setText(s.right(1).toUpper());
}
