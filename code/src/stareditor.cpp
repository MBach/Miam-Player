/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
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
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
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
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtGui>

#include "stareditor.h"
#include "starrating.h"

#include <QDebug>

StarEditor::StarEditor(QWidget *parent)
	: QWidget(parent)
{
	setMouseTracking(true);
	//TODO : remove bg filled ?
	setAutoFillBackground(true);
}

QSize StarEditor::sizeHint() const
{
	return star.sizeHint();
}

void StarEditor::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	const QRect r(rect());
	//qDebug() << "StarEditor::paintEvent" << r;
	star.paint(&painter, &r, this->palette(), StarRating::Editable);
}

void StarEditor::mouseMoveEvent(QMouseEvent *event)
{
	int istar = starAtPosition(event->x());

	if (istar != star.starCount() && istar != -1) {
		star.setStarCount(istar);
		//qDebug() << "star.starCount():" << star.starCount();
		update();
	}
}

void StarEditor::mouseReleaseEvent(QMouseEvent *event /* event */)
{
	//qDebug() << rect().contains(event->pos());
	emit editingFinished(this);
}

int StarEditor::starAtPosition(int x)
{
	int istar = (x / (star.sizeHint().width() / star.max())) + 1;
	if (istar < 0 || istar > star.max()) {
		return -1;
	}
	return istar;
}
