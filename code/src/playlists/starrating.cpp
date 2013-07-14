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

void StarRating::paint(QPainter *painter, const QRect &rect, const QPalette &palette, EditMode mode) const
{
	painter->save();

	painter->setRenderHint(QPainter::Antialiasing, true);

	/// XXX: extract this somewhere?
	QPen pen(QColor(171, 122, 77));
	QLinearGradient linearGradientBrush(0, 0, 0, 1);
	QLinearGradient linearGradientPen(0, 0, 0, 1);
	QLinearGradient backgroundGradient(0, 0, 0, rect.height());

	pen.setWidthF(pen.widthF() / rect.height());

	switch (mode) {
	case Editable:
		#if defined(Q_OS_WIN)
		///XXX
		backgroundGradient.setColorAt(0, QColor::fromRgb(221, 236, 251));
		backgroundGradient.setColorAt(1, QColor::fromRgb(202, 224, 251));
		painter->fillRect(rect, QBrush(backgroundGradient));

		linearGradientBrush.setColorAt(0, Qt::white);
		linearGradientBrush.setColorAt(1, QColor(253, 230, 116));

		linearGradientPen.setColorAt(0, QColor(227, 178, 94));
		linearGradientPen.setColorAt(1, QColor(166, 122, 87));

		pen.setColor(QColor(171, 122, 77));
		pen.setBrush(QBrush(linearGradientPen));
		#elif defined(Q_OS_UNIX)
		linearGradientBrush.setColorAt(0, Qt::white);
		linearGradientBrush.setColorAt(1, QColor(242, 122, 73));
		#endif
		painter->setBrush(QBrush(linearGradientBrush));
		break;
	case NoStarsYet:
		painter->setBrush(QBrush(QColor::fromRgbF(1, 1, 1, 0.9)));
		break;
	case ReadOnly:
		#if defined(Q_OS_WIN)
		linearGradientBrush.setColorAt(0, Qt::white);
		linearGradientBrush.setColorAt(1, QColor(253, 230, 116));

		linearGradientPen.setColorAt(0, QColor(227, 178, 94));
		linearGradientPen.setColorAt(1, QColor(166, 122, 87));

		pen.setColor(QColor(171, 122, 77));
		pen.setBrush(QBrush(linearGradientPen));
		#elif defined(Q_OS_UNIX)
		linearGradientBrush.setColorAt(0, Qt::white);
		linearGradientBrush.setColorAt(1, QColor(242, 122, 73));
		#endif
		painter->setBrush(QBrush(linearGradientBrush));
		break;
	}
	painter->setPen(pen);

	int yOffset = (rect.height() - rect.height() * starPolygon.boundingRect().height()) / 2;
	painter->translate(rect.x(), rect.y() + yOffset);
	if (rect.height() < rect.width() / 5) {
		painter->scale(rect.height(), rect.height());
	} else {
		painter->scale(rect.width() / maxStarCount, rect.width() / maxStarCount);
	}

	for (int i = 0; i < maxStarCount; ++i) {
		if (i < _starCount) {
			painter->drawPolygon(starPolygon);
		} else if (mode == Editable) {
			painter->drawPolygon(diamondPolygon, Qt::WindingFill);
		}
		painter->translate(1.0, 0);
	}

	painter->restore();
}
