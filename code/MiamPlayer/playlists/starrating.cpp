/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtWidgets>
#include <math.h>

#include "starrating.h"

int StarRating::maxStarCount = 5;

StarRating::StarRating(int starCount)
{
	_starCount = starCount;
	for (int i = 0; i < 5; ++i) {
		QLineF l(0.5, 0.5, 0.5, 0);
		l.setAngle(i * 72 + 18);
		starPolygon << l.p2();

		QLineF l2(0.5, 0.5, 0.5, 0.71);
		l2.setAngle(i * 72 + 54);
		starPolygon << l2.p2();
	}
	diamondPolygon << QPointF(0.4, 0.5) << QPointF(0.5, 0.4) << QPointF(0.6, 0.5) << QPointF(0.5, 0.6) << QPointF(0.4, 0.5);
}

void StarRating::setStarCount(int starCount)
{
	if (starCount < 0) {
		_starCount = 0;
	} else if (starCount > maxStarCount) {
		_starCount = maxStarCount;
	} else {
		_starCount = starCount;
	}
}

void StarRating::paintStars(QPainter *painter, const QStyleOptionViewItem &o, EditMode mode) const
{
	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);

	/// XXX: extract this somewhere?
	QColor penColor(171, 122, 77);
	QPen pen(penColor);
	QLinearGradient linearGradientBrush(0, 0, 0, 1);
	QLinearGradient linearGradientPen(0, 0, 0, 1);

	QStyleOptionViewItem opt(o);
	opt.rect.adjust(0, 1, 0, -1);

	pen.setWidthF(pen.widthF() / opt.rect.height());

	if (_starCount == 0 && mode == ReadOnly && opt.state.testFlag(QStyle::State_Selected)) {
		mode = NoStarsYet;
	}
	switch (mode) {
	case Editable:
		painter->fillRect(opt.rect, QApplication::style()->standardPalette().highlight().color().lighter());

		linearGradientBrush.setColorAt(0, Qt::white);
		linearGradientBrush.setColorAt(1, QColor(253, 230, 116));

		linearGradientPen.setColorAt(0, QColor(227, 178, 94));
		linearGradientPen.setColorAt(1, QColor(166, 122, 87));

		pen.setColor(penColor);
		pen.setBrush(QBrush(linearGradientPen));
		painter->setBrush(QBrush(linearGradientBrush));
		break;
	case NoStarsYet:
		pen.setColor(penColor.lighter(135));
		painter->setBrush(QApplication::style()->standardPalette().highlight().color().lighter(165));
		break;
	case ReadOnly:
		linearGradientBrush.setColorAt(0, Qt::white);
		linearGradientBrush.setColorAt(1, QColor(253, 230, 116));

		linearGradientPen.setColorAt(0, QColor(227, 178, 94));
		linearGradientPen.setColorAt(1, QColor(166, 122, 87));

		pen.setColor(penColor);
		pen.setBrush(QBrush(linearGradientPen));
		painter->setBrush(QBrush(linearGradientBrush));
		break;
	}
	painter->setPen(pen);

	int yOffset = (opt.rect.height() - opt.rect.height() * starPolygon.boundingRect().height()) / 2;
	painter->translate(opt.rect.x(), opt.rect.y() + yOffset);
	if (opt.rect.height() < opt.rect.width() / 5) {
		painter->scale(opt.rect.height(), opt.rect.height());
	} else {
		painter->scale(opt.rect.width() / maxStarCount, opt.rect.width() / maxStarCount);
	}

	for (int i = 0; i < maxStarCount; ++i) {
		if (i < _starCount || mode == NoStarsYet) {
			painter->drawPolygon(starPolygon);
		} else if (mode == Editable) {
			painter->drawPolygon(diamondPolygon, Qt::WindingFill);
		}
		painter->translate(1.0, 0);
	}
	painter->restore();
}
