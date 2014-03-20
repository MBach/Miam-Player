#include "addressbarbutton.h"

#include <QDir>
#include <QMouseEvent>
#include <QStyleOption>
#include <QStylePainter>

#include <QtDebug>

AddressBarButton::AddressBarButton(const QString &newPath, int index, QWidget *parent) :
	QPushButton(parent), _path(QDir::toNativeSeparators(newPath)), idx(index), _atLeastOneSubDir(true)
{
	if (_path.right(1) != QDir::separator()) {
		_path += QDir::separator();
	}
	this->setFlat(true);
	this->setMouseTracking(true);
	QDir d = QDir(_path);
	foreach (QFileInfo fileInfo, d.entryInfoList(QStringList(), QDir::NoDotAndDotDot)) {
		qDebug() << fileInfo.absoluteFilePath();
		if (fileInfo.isDir()) {
			_atLeastOneSubDir = true;
			break;
		}
	}
	// padding + text + (space + arrow + space) if current dir has subdirectories
	int width = fontMetrics().width(d.dirName());
	if (_atLeastOneSubDir) {
		width += 40;
	}
	this->setMinimumWidth(width);
	this->setMaximumWidth(width);
}

QString AddressBarButton::currentPath() const
{
	// Removes the last directory separator, unless for the root on windows which is like C:\. It's not possible to do "cd C:"
	if (QDir(_path).isRoot()) {
		return _path;
	} else {
		return _path.left(_path.length() - 1);
	}
}

/** Redefined. */
void AddressBarButton::mouseMoveEvent(QMouseEvent *)
{
	qobject_cast<QWidget *>(this->parent())->repaint();
	qDebug() << "over" << _path;
}

/** Redefined. */
void AddressBarButton::mousePressEvent(QMouseEvent *event)
{
	// mouse press in arrow rect => immediate popup menu
	// in text rect => goto dir, mouse release not relevant

	if (_arrowRect.contains(event->pos())) {
		//_arrowPressed = true;
		emit aboutToShowMenu();
	}
}

#include <QApplication>
#include "settings.h"

/** Redefined. */
void AddressBarButton::paintEvent(QPaintEvent *)
{
	QStylePainter p(this);
	QRect r = rect().adjusted(0, 1, -1, -1);

	QPalette palette = QApplication::palette();
	QLinearGradient g(rect().topLeft(), rect().bottomLeft());
	if (Settings::getInstance()->isCustomColors()) {
		g.setColorAt(0, palette.base().color().lighter(110));
		g.setColorAt(1, palette.base().color());
	} else {
		g.setColorAt(0, palette.base().color());
		g.setColorAt(1, palette.window().color());
	}
	p.fillRect(r, g);
	if (_atLeastOneSubDir) {
		_arrowRect = QRect(r.width() - 15, r.y(), 15, r.height());
		_textRect = QRect(r.x(), r.y(), r.width() - 15, r.height());
	} else {
		_textRect = r;
	}
	QDir dir(_path);

	QPoint pos = mapFromGlobal(QCursor::pos());
	p.save();
	if (_textRect.contains(pos)) {
		p.setPen(QApplication::palette().highlight().color());
		p.setBrush(QApplication::palette().highlight().color().lighter());
		p.drawRect(_textRect);
		if (_atLeastOneSubDir) {
			p.drawRect(_arrowRect);
		}
	} else if (_arrowRect.contains(pos)) {
		p.setPen(QApplication::palette().highlight().color());
		p.drawRect(_textRect);
		p.setBrush(QApplication::palette().highlight().color().lighter());
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
		if (isLeftToRight()) {
			p.drawPrimitive(QStyle::PE_IndicatorArrowRight, o);
		} else {
			p.drawPrimitive(QStyle::PE_IndicatorArrowLeft, o);
		}
	}

	if (dir.isRoot()) {
		bool absRoot = true;
		foreach (QFileInfo fileInfo, QDir::drives()) {
			if (QDir::toNativeSeparators(fileInfo.absolutePath()) == _path) {
				p.drawText(_textRect, Qt::AlignLeft | Qt::AlignVCenter, _path);
				absRoot = false;
				break;
			}
		}
		if (absRoot) {
			p.drawPixmap(QRect(0, 5, 20, 20), style()->standardPixmap(QStyle::SP_ComputerIcon), QRect(0, 0, 20, 20));
		}
	} else {
		if (!dir.dirName().isEmpty()) {
			p.drawText(_textRect.adjusted(5, 0, 0, 0), Qt::AlignLeft | Qt::AlignVCenter, dir.dirName());
		}
	}
}
