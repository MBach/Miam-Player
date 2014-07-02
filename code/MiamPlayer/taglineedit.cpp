#include "taglineedit.h"
#include "settings.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QStylePainter>
#include <QStyleOptionFrameV3>

#include <QtDebug>

TagLineEdit::TagLineEdit(QWidget *parent) :
	LineEdit(parent), autoTransform(false), _timerTag(new QTimer(this))
{
	_timerTag->setInterval(1000);
	_timerTag->setSingleShot(true);

	connect(_timerTag, &QTimer::timeout, this, &TagLineEdit::createTag);

	this->installEventFilter(this);

	QHBoxLayout *b = new QHBoxLayout(this);
	b->setContentsMargins(0, 0, 0, 1);
	b->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
}

void TagLineEdit::addTag(const QString &tag)
{
	bool found = false;
	foreach (TagButton *button, _tags) {
		if (button->text() == tag.trimmed().toLower()) {
			found = true;
			break;
		}
	}

	// It useless to add a tag more than once (IMHO)
	if (found) {
		return;
	}

	TagButton *t = new TagButton(tag.trimmed(), this);
	t->setMaximumHeight(this->height() - 2);
	connect(t->closeButton(), &QToolButton::clicked, this, [=]() {
		_tags.removeOne(t);
		t->deleteLater();
		emit taglistHasChanged(this->toStringList());
	});
	_tags.append(t);
	this->layout()->addWidget(t);
}

bool TagLineEdit::isAutoTransform() const
{
	return autoTransform;
}

void TagLineEdit::setAutoTransform(bool enabled)
{
	autoTransform = enabled;
}

bool TagLineEdit::eventFilter(QObject *obj, QEvent *event)
{
	if (autoTransform) {
		if (obj == this && event->type() == QEvent::KeyRelease) {
			_timerTag->start();
		} else if (obj == this && event->type() == QEvent::KeyPress) {
			if (_timerTag->isActive()) {
				_timerTag->start();
			}
		}
	}
	/// Todo !autoTransform
	/*if (obj == this && event->type() == QEvent::FocusIn) {

	}*/
	return QLineEdit::eventFilter(obj, event);
}

/** Redefined to display user input like closable "bubbles". */
void TagLineEdit::paintEvent(QPaintEvent *)
{
	QStylePainter p(this);

	// Draw frame
	QStyleOptionFrameV2 frame;
	this->initStyleOption(&frame);
	QPalette palette = QApplication::palette();
	p.setPen(palette.mid().color());
	p.setBrush(palette.base());
	p.drawRect(this->rect().adjusted(0, 0, -1, -1));

	// Compute cursor position
	QRect contentsRect = this->style()->subElementRect(QStyle::SE_LineEditContents, &frame);
	int w = 0;
	foreach (TagButton *tag, _tags) {
		w += tag->width() + this->layout()->spacing();
	}
	QRect rText = contentsRect.adjusted(w + 2, 0, 0, 0);
	if (w == 0 && !hasFocus()) {
		p.setPen(palette.mid().color());
		p.drawText(rText, Qt::AlignLeft | Qt::AlignVCenter, placeholderText());
	} else {
		p.setPen(palette.text().color());
		p.drawText(rText, Qt::AlignLeft | Qt::AlignVCenter, text());

		// Animate cursor is focus is owned by this widget
		if (hasFocus()) {
			this->drawCursor(&p, rText.adjusted(0, 1, 0, -1));
		}
	}
}

QStringList TagLineEdit::toStringList() const
{
	QStringList tags;
	foreach (TagButton *b, _tags) {
		tags << b->text();
	}
	return tags;
}

void TagLineEdit::createTag()
{
	if (!this->text().trimmed().isEmpty()) {
		this->addTag(this->text());
		this->clear();

		emit taglistHasChanged(this->toStringList());
	}
}
