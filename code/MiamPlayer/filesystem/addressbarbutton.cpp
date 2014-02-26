#include "addressbarbutton.h"

#include <QDir>
#include <QStyleOption>
#include <QStylePainter>

#include <QtDebug>

AddressBarButton::AddressBarButton(const QString &newPath, int index, QWidget *parent) :
	QPushButton(parent), path(QDir::toNativeSeparators(newPath)), idx(index), _atLeastOneSubDir(false)
{
	if (path.right(1) != QDir::separator()) {
		path += QDir::separator();
	}
	this->setFlat(true);
	this->setMouseTracking(true);
	QDir d = QDir(path);
	foreach (QFileInfo fileInfo, d.entryInfoList()) {
		if (fileInfo.isDir()) {
			_atLeastOneSubDir = true;
			break;
		}
	}
	// padding + text + (space + arrow + space) if current dir has subdirectories
	int width = fontMetrics().width(d.dirName());
	if (_atLeastOneSubDir) {
		width += 30;
	}
	this->setMinimumWidth(width);
	this->setMaximumWidth(width);
}

QString AddressBarButton::currentPath() const
{
	// Removes the last directory separator, unless for the root on windows which is like C:\. It's not possible to do "cd C:"
	if (QDir(path).isRoot()) {
		return path;
	} else {
		return path.left(path.length() - 1);
	}
}

/** Redefined. */
void AddressBarButton::mouseMoveEvent(QMouseEvent *)
{
	qobject_cast<QWidget *>(this->parent())->repaint();
}

/** Redefined. */
void AddressBarButton::mousePressEvent(QMouseEvent *)
{
	// mouse press in arrow rect => immediate popup menu
	// in text rect => goto dir, mouse release not relevant
}

/** Redefined. */
void AddressBarButton::paintEvent(QPaintEvent *)
{
	QStylePainter p(this);
	QRect r = rect().adjusted(0, 1, -1, -2);
	_textRect = QRect(r.x(), r.y(), r.width() - 15, r.height());
	if (_atLeastOneSubDir) {
		_arrowRect = QRect(r.width() - 15, r.y(), 15, r.height());
	}
	QDir dir(path);

	QPoint pos = mapFromGlobal(QCursor::pos());
	p.save();
	if (_textRect.contains(pos)) {
		p.setPen(palette().highlight().color());
		p.setBrush(palette().highlight().color().lighter());
		p.drawRect(_textRect);
		if (_atLeastOneSubDir) {
			p.drawRect(_arrowRect);
		}
	} else if (_arrowRect.contains(pos)) {
		p.setPen(palette().highlight().color());
		p.drawRect(_textRect);
		p.setBrush(palette().highlight().color().lighter());
		if (_atLeastOneSubDir) {
			p.drawRect(_arrowRect);
		}
	} else {
		p.setPen(Qt::NoPen);
		p.setBrush(Qt::NoBrush);
		p.drawRect(_textRect);
		if (_atLeastOneSubDir) {
			p.drawRect(_arrowRect);
		}
	}
	p.restore();

	if (_atLeastOneSubDir) {
		QStyleOptionButton o;
		o.initFrom(this);
		o.rect = _arrowRect.adjusted(4, 7, -2, -4);
		p.drawPrimitive(QStyle::PE_IndicatorArrowRight, o);
	}

	if (dir.isRoot()) {
		bool absRoot = true;
		foreach (QFileInfo fileInfo, QDir::drives()) {
			if (QDir::toNativeSeparators(fileInfo.absolutePath()) == path) {
				p.drawText(_textRect, Qt::AlignLeft | Qt::AlignVCenter, path);
				absRoot = false;
				break;
			}
		}
		if (absRoot) {
			p.drawPixmap(QRect(0, 0, 20, 20), style()->standardPixmap(QStyle::SP_ComputerIcon), QRect(0, 0, 20, 20));
		}
	} else {
		if (!dir.dirName().isEmpty()) {
			p.drawText(_textRect.adjusted(5, 0, 0, 0), Qt::AlignLeft | Qt::AlignVCenter, dir.dirName());
		}
	}
}
