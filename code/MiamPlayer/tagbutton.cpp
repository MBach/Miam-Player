#include "tagbutton.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QToolButton>

#include <QtDebug>

TagButton::TagButton(const QString &tag, QWidget *parent) :
	QWidget(parent), _label(new QLabel(tag, this)), _closeButton(new QToolButton(this))
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

void TagButton::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	p.setPen(QApplication::palette().mid().color());
	p.setBrush(Qt::NoBrush);
	p.drawRect(this->rect().adjusted(2, 2, -1, -1));
}
