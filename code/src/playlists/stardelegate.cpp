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

#include "stardelegate.h"
#include "stareditor.h"
#include "starrating.h"

#include "settings.h"

StarDelegate::StarDelegate(QWidget *parent)
	: QStyledItemDelegate(parent)
{

}

/** Redefined. */
QWidget *StarDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const
{
	StarEditor *editor = new StarEditor(parent);
	connect(editor, &StarEditor::editingFinished, [=]() {
		emit commitData(editor);
		delete editor;
	});
	return editor;
}

/** Redefined. */
void StarDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	// Removes the dotted rectangle
	QStyleOptionViewItem opt = option;
	opt.state &= ~QStyle::State_HasFocus;

	///XXX
	QLinearGradient linearGradient(opt.rect.x(), opt.rect.y(), opt.rect.x(), opt.rect.y() + opt.rect.height());
	linearGradient.setColorAt(0, QColor::fromRgb(221, 236, 251));
	linearGradient.setColorAt(1, QColor::fromRgb(202, 224, 251));
	QBrush brush(linearGradient);

	if (opt.state & QStyle::State_Selected) {
		painter->fillRect(opt.rect, brush);
	}
	if (index.data().canConvert<StarRating>()) {
		StarRating starRating = qvariant_cast<StarRating>(index.data());
		starRating.paint(painter, opt.rect, opt.palette, StarRating::ReadOnly);
	} else if (opt.state & QStyle::State_Selected) {
		StarRating starRating(StarRating::maxStarCount);
		starRating.paint(painter, opt.rect, opt.palette, StarRating::NoStarsYet);
	}
}

/** Redefined. */
void StarDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
	qDebug() << Q_FUNC_INFO << editor;
	StarRating starRating = qvariant_cast<StarRating>(index.data());
	StarEditor *starEditor = qobject_cast<StarEditor *>(editor);
	starEditor->setStarRating(starRating);
}

/** Redefined. */
void StarDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
	qDebug() << Q_FUNC_INFO;
	StarEditor *starEditor = qobject_cast<StarEditor *>(editor);
	model->setData(index, QVariant::fromValue(starEditor->starRating()));
}

/** Redefined. */
QSize StarDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if (index.data().canConvert<StarRating>()) {
		return option.rect.size();
	} else {
		return QStyledItemDelegate::sizeHint(option, index);
	}
}
