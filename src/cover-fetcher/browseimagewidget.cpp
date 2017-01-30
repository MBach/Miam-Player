#include "browseimagewidget.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QPainter>
#include <QToolButton>
#include <QVBoxLayout>

BrowseImageWidget::BrowseImageWidget(QStackedWidget *parent)
	: QWidget(parent)
{
	QHBoxLayout *hbox = new QHBoxLayout;
	QToolButton *left = new QToolButton(this);
	left->setArrowType(Qt::LeftArrow);
	QToolButton *right = new QToolButton(this);
	right->setArrowType(Qt::RightArrow);

	hbox->addWidget(left);
	hbox->addSpacerItem(new QSpacerItem(20, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));
	hbox->addWidget(right);

	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->addSpacerItem(new QSpacerItem(1, 20, QSizePolicy::Fixed, QSizePolicy::Expanding));
	vbox->addLayout(hbox);
	setLayout(vbox);

	connect(left, &QToolButton::pressed, this, [=]() {
		int i = parent->currentIndex();
		if (i == 0) {
			parent->setCurrentIndex(parent->count() - 1);
		} else {
			parent->setCurrentIndex(--i);
		}
	});
	connect(right, &QToolButton::pressed, this, [=]() {
		int i = parent->currentIndex();
		if (i + 1 == parent->count()) {
			parent->setCurrentIndex(0);
		} else {
			parent->setCurrentIndex(++i);
		}
	});
}

void BrowseImageWidget::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	int h = 45;
	QRect r(rect().x(), rect().y() + rect().height() - h, rect().width(), h);
	QBrush brush(QColor(0, 0, 0, 128));
	p.setBrush(brush);
	p.setPen(Qt::NoPen);
	p.drawRect(r);
}
