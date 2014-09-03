#include "tagbutton.h"


#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QShowEvent>
#include <QToolButton>

#include "taglineedit.h"

#include <QtDebug>

TagButton::TagButton(const QString &tag, TagLineEdit *parent) :
	QWidget(parent), _tagLineEdit(parent), _label(new QLabel(tag, this)), _closeButton(new QToolButton(this)),
	_position(-1), _spaceCount(-1), _column(-1)
{
	_closeButton->setAutoRaise(true);
	_closeButton->setIconSize(QSize(14, 14));
	_closeButton->setIcon(QIcon(":/icons/closeTabs"));

	QHBoxLayout *hBoxLayout = new QHBoxLayout(this);
	hBoxLayout->setContentsMargins(3, 3, 0, 0);
	hBoxLayout->setSpacing(3);
	this->setLayout(hBoxLayout);
	_label->setIndent(2);
	hBoxLayout->addWidget(_label);
	hBoxLayout->addWidget(_closeButton);

	this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

/*QSize TagButton::sizeHint() const
{
	int wSpace = _tagLineEdit->fontMetrics().width(" ");
	int nbOfSpaces = ceil(width() / (double) wSpace);
	return QSize(nbOfSpaces * wSpace, height());
}*/

void TagButton::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	p.setPen(QApplication::palette().mid().color());
	p.setBrush(Qt::NoBrush);
	p.drawRect(this->rect().adjusted(2, 2, -1, -1));
}

void TagButton::showEvent(QShowEvent *event)
{
	QWidget::showEvent(event);
	emit shown();
}
