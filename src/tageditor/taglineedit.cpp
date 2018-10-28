#include "taglineedit.h"
#include "settings.h"

#include <QApplication>
#include <QKeyEvent>
#include <QStylePainter>
#include <QStyleOptionFrame>

#include <QtDebug>

TagLineEdit::TagLineEdit(QWidget *parent) :
	LineEdit(parent)
{
	connect(this, &TagLineEdit::textEdited, this, &TagLineEdit::clearTextAndTags);

	this->installEventFilter(this);
}

void TagLineEdit::addTag(const QString &tag, int column)
{
	if (tag.trimmed().isEmpty()) {
		return;
	}
	for (TagButton *button : _tags) {
		if (button->text() == tag.trimmed().toLower()) {
			// It useless to add a tag more than once (IMHO)
			return;
		}
	}

	TagButton *t = new TagButton(tag.trimmed(), this);
	if (column != -1) {
		t->setColumn(column);
	}
	t->setMaximumHeight(this->height() - 2);

	// Move all tag buttons, next to the one that is about to be closed, to the left
	connect(t->closeButton(), &QToolButton::clicked, this, [=]() {
		this->closeTagButton(t);
	});

	_tags.append(t);
	this->setFocus();

	// Unfortunately, we have to wait that a QShowEvent is emitted to have correct size of the Widget
	connect(t, &TagButton::shown, this, &TagLineEdit::insertSpaces);
	t->move(cursorRect().right() + 1, 0);
	t->show();
}

/** Redefined to be able to move tag buttons.
 * Backspace method is not virtual in QLineEdit, therefore keyPressEvent must be intercepted and eaten. */
void TagLineEdit::backspace()
{
	bool oneTagNeedToBeRemoved = false;
	TagButton *tag = nullptr;
	QPoint cursorCenter;

	cursorBackward(false);
	int dx = fontMetrics().width(text().at(cursorPosition()));
	for (TagButton *button : _tags) {
		cursorCenter = cursorRect().center();
		// One tag button need to be removed
		if (button->frameGeometry().contains(cursorCenter)) {
			oneTagNeedToBeRemoved = true;
			tag = button;
		}

		// Tags need to be moved to the left
		if (button->x() > cursorCenter.x()) {
			button->move(button->x() - dx, 0);
			button->setPosition(cursorPosition());
		}
	}

	cursorForward(false);
	if (oneTagNeedToBeRemoved) {
		for (int i = 0; i < tag->spaceCount(); i++) {
			LineEdit::backspace();
		}

		dx = fontMetrics().width(" ") * tag->spaceCount();
		for (TagButton *button : _tags) {
			if (button != tag && button->x() > cursorCenter.x()) {
				button->move(button->x() - dx, 0);
				button->setPosition(cursorPosition());
			}
		}
		_tags.removeOne(tag);
		delete tag;
		/// FIXME
		//emit taglistHasChanged(this->toStringList());
	} else {
		LineEdit::backspace();
	}
}

bool TagLineEdit::eventFilter(QObject *obj, QEvent *event)
{
	if (obj == this && event->type() == QEvent::KeyPress) {
		QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
		if (keyEvent->key() == Qt::Key_Backspace) {
			this->backspace();
			// Ate Backspace key event
			return true;
		}
	}
	return LineEdit::eventFilter(obj, event);
}

/** Redefined to be able to move TagButton when typing. */
void TagLineEdit::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right) {
		/// TODO cursorWordForward / cursorWordBackard to stop on TagButton
		if (QGuiApplication::keyboardModifiers().testFlag(Qt::ControlModifier)) {
			qDebug() << "cursorWordForward / backward";
		}
		LineEdit::keyPressEvent(event);
		for (TagButton *t : _tags) {
			if (t->frameGeometry().contains(cursorRect().center())) {
				if (event->key() == Qt::Key_Left) {
					cursorBackward(false, t->spaceCount() - 1);
				} else {
					cursorForward(false, t->spaceCount() - 1);
				}
				break;
			}
		}
	} else {
		QString k = event->text();
		int w = fontMetrics().width(k);
		if (event->key() == Qt::Key_Delete) {
			w = -w;
		}
		for (TagButton *t : _tags) {
			if (t->frameGeometry().x() > cursorRect().center().x()) {
				t->move(t->x() + w, 0);
			}
		}
		LineEdit::keyPressEvent(event);
	}
}

/** Redefined to automatically move cursor outside TagButton. */
void TagLineEdit::mousePressEvent(QMouseEvent *event)
{
	LineEdit::mousePressEvent(event);
	for (TagButton *t : _tags) {
		QRect r = t->frameGeometry();
		if (r.contains(event->pos())) {
			if (r.x() + r.width() / 2 >= event->pos().x()) {
				while (r.contains(cursorRect().center()) && cursorPosition() > 0) {
					cursorBackward(false);
				}
			} else {
				while (r.contains(cursorRect().center()) && cursorPosition() < text().length()) {
					cursorForward(false);
				}
			}
			break;
		}
	}
}

/** Redefined to display user input like closable "bubbles". */
void TagLineEdit::paintEvent(QPaintEvent *)
{
	QStylePainter p(this);

	// Draw frame
    QStyleOptionFrame frame;
	this->initStyleOption(&frame);
	QPalette palette = QApplication::palette();
	p.setPen(palette.mid().color());
	p.setBrush(palette.base());
	p.drawRect(this->rect().adjusted(0, 0, -1, -1));

	// Compute cursor position
	QRect contentsRect = this->style()->subElementRect(QStyle::SE_LineEditContents, &frame);
	QRect rText = contentsRect.adjusted(2, 0, 0, 0);
	if (!_tags.isEmpty() || (placeholderText().isEmpty() || (!placeholderText().isEmpty() && hasFocus()))) {
		p.setPen(palette.text().color());
		p.drawText(rText, Qt::AlignLeft | Qt::AlignVCenter, text());

		// Animate cursor is focus is owned by this widget
		bool overlap = false;
		for (TagButton *t : _tags) {
			if (t->frameGeometry().contains(cursorRect().center())) {
				overlap = true;
				break;
			}
		}
		if (!overlap && hasFocus()) {
			this->drawCursor(&p, rText.adjusted(0, 1, 0, -1));
		}
	} else {
		p.setPen(palette.mid().color());
		p.drawText(rText, Qt::AlignLeft | Qt::AlignVCenter, placeholderText());
	}
}

QStringList TagLineEdit::toStringList() const
{
	QStringList tags;
	for (TagButton *b : _tags) {
		tags << b->text();
	}
	return tags;
}

void TagLineEdit::closeTagButton(TagButton *t)
{
	int spaces = t->spaceCount();
	this->setText(text().remove(t->position(), t->spaceCount()));
	for (TagButton *otherTag : _tags) {
		if (otherTag != t) {
			if (otherTag->position() > t->position()) {
				int dx = fontMetrics().width(" ") * t->spaceCount();
				otherTag->move(otherTag->x() - dx, 0);
			}
		}
	}
	_tags.removeOne(t);
	for (TagButton *otherTag : _tags) {
		otherTag->setPosition(otherTag->position() - spaces);
	}
	t->deleteLater();
}

void TagLineEdit::clearTextAndTags(const QString &txt)
{
	if (txt.isEmpty()) {
		for (TagButton *tag : _tags) {
			tag->deleteLater();
		}
		_tags.clear();
	}
}

/** TagButton instances are converted with whitespaces in the LineEdit in order to move them. */
void TagLineEdit::insertSpaces()
{
	TagButton *t = qobject_cast<TagButton*>(sender());
	int cx = cursorRect().x();
	t->setPosition(cursorPosition());
	int numberOfSpace = 2;

	this->setText(this->text().insert(cursorPosition(), "  "));

	cursorForward(false, 2);
	while (t->frameGeometry().contains(cursorRect().center())) {
		this->setText(this->text().insert(cursorPosition(), " "));
		cursorForward(false);
		numberOfSpace++;
	}
	t->setMinimumWidth(numberOfSpace * fontMetrics().width(" ") - 5);
	t->setSpaceCount(numberOfSpace);
	t->disconnect();

	for (TagButton *tag : _tags) {
		//qDebug() << Q_FUNC_INFO << "trying to move tag";
		if (t != tag && tag->frameGeometry().x() > cx) {
			//qDebug() << Q_FUNC_INFO << "moving tag" << tag->text();
			tag->move(tag->x() + fontMetrics().width(" ") * numberOfSpace, 0);
		}
	}
}
