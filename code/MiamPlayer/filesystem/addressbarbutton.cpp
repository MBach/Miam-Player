#include "addressbarbutton.h"

#include "addressbar.h"

#include "settingsprivate.h"
#include <QApplication>
#include <QDirIterator>
#include <QFileIconProvider>
#include <QMouseEvent>
#include <QStyleOption>
#include <QStylePainter>

#include <QtDebug>

AddressBarButton::AddressBarButton(const QDir &newPath, AddressBar *parent) :
	QPushButton(parent), _path(newPath),
	_atLeastOneSubDir(false), _highlighted(false), _addressBar(parent)
{
	this->setFlat(true);
	this->setMouseTracking(true);

	QDir d = QDir(_path);
	QDirIterator it(d.absolutePath(), QDir::Dirs | QDir::NoDotAndDotDot);
	while (it.hasNext()) {
		it.next();
		if (it.fileInfo().isDir()) {
			_atLeastOneSubDir = true;
			break;
		}
	}

	// Text + (space + arrow + space) if current dir has subdirectories
	int width = fontMetrics().width(d.dirName());
	if (_atLeastOneSubDir) {
		// Special root folders like "/" or "D:\" are empty
		if (d.dirName().isEmpty()) {
			width += 40;
		} else {
			width += 25;
		}
		this->setMinimumWidth(width);
	} else {
		this->setMinimumWidth(width + 15);
	}
	this->setText(d.dirName());
	this->setMaximumWidth(width);
}

void AddressBarButton::setHighlighted(bool b)
{
	_highlighted = b;
	if (b) {
		emit aboutToShowMenu();
	}
	repaint();
}

/** Redefined. */
void AddressBarButton::mouseMoveEvent(QMouseEvent *)
{
	qobject_cast<QWidget *>(this->parent())->update();
}

/** Redefined. */
void AddressBarButton::mousePressEvent(QMouseEvent *event)
{
	// mouse press in arrow rect => immediate popup menu
	// in text rect => goto dir, mouse release event is not relevant
	if (_arrowRect.contains(event->pos())) {
		this->setHighlighted(true);
	} else if (_textRect.contains(event->pos())) {
		_addressBar->init(_path);
	}
}

/** Redefined. */
void AddressBarButton::paintEvent(QPaintEvent *)
{
	QStylePainter p(this);
	QRect r = rect().adjusted(0, 1, -1, -1);

	QPalette palette = QApplication::palette();
	QLinearGradient g(rect().topLeft(), rect().bottomLeft());
	if (SettingsPrivate::instance()->isCustomColors()) {
		g.setColorAt(0, palette.base().color().lighter(110));
		g.setColorAt(1, palette.base().color());
	} else {
		g.setColorAt(0, palette.base().color());
		g.setColorAt(1, palette.window().color());
	}
	p.fillRect(r, g);

	QDir dir(_path);
	if (_atLeastOneSubDir) {
		if (isLeftToRight()) {
			_arrowRect = QRect(r.width() - 15, r.y(), 15, r.height());
			_textRect = QRect(r.x(), r.y(), r.width() - 15, r.height());
		} else {
			_arrowRect = QRect(r.x(), r.y(), 15, r.height());
			_textRect = QRect(r.x() + 15, r.y(), r.width() - 15, r.height());
		}
	} else {
		_textRect = r.adjusted(0, 0, -5, 0);
	}

	QPoint pos = mapFromGlobal(QCursor::pos());
	p.save();
	if (_highlighted || _textRect.contains(pos)) {
		p.setPen(palette.highlight().color());
		p.setBrush(palette.highlight().color().lighter());
		p.drawRect(_textRect);
	} else if (_highlighted || _arrowRect.contains(pos)) {
		p.setPen(palette.highlight().color());
		p.drawRect(_textRect);
		p.setBrush(palette.highlight().color().lighter());
	} else {
		p.setPen(Qt::NoPen);
		p.setBrush(Qt::NoBrush);
		p.drawRect(_textRect);
	}
	if (_atLeastOneSubDir) {
		p.drawRect(_arrowRect);
	}
	p.restore();

	if (_atLeastOneSubDir) {
		QStyleOptionButton o;
		o.initFrom(this);
		if (isLeftToRight()) {
			o.rect = _arrowRect.adjusted(4, 7, -2, -4);
		} else {
			o.rect = _arrowRect.adjusted(2, 7, -4, -4);
		}
		/// TODO subclass for root button with special arrow when folders are hidden?
		p.setBrush(o.palette.highlight());
		if (_highlighted) {
			p.drawPrimitive(QStyle::PE_IndicatorArrowDown, o);
		} else if (isLeftToRight()) {
			p.drawPrimitive(QStyle::PE_IndicatorArrowRight, o);
		} else {
			p.drawPrimitive(QStyle::PE_IndicatorArrowLeft, o);
		}
	}

	if (_highlighted || _textRect.contains(pos)) {
		p.setPen(palette.highlightedText().color());
	} else if (_highlighted || _arrowRect.contains(pos)) {
		p.setPen(palette.windowText().color());
	} else {
		p.setPen(palette.windowText().color());
	}
	if (dir.isRoot()) {
		bool absRoot = true;
		foreach (QFileInfo fileInfo, QDir::drives()) {
			if (fileInfo.absolutePath() == _path.absolutePath()) {
				p.drawText(_textRect.adjusted(5, 0, 0, 0), Qt::AlignLeft | Qt::AlignVCenter, QDir::toNativeSeparators(_path.absolutePath()));
				absRoot = false;
				break;
			}
		}
		if (absRoot) {
			QPixmap computer = QFileIconProvider().icon(QFileIconProvider::Computer).pixmap(20, 20);
			if (isLeftToRight()) {
				p.drawPixmap(2, 3, 20, 20, computer);
			} else {
				p.drawPixmap(18, 3, 20, 20, computer);
			}
		}
	} else {
		if (!dir.dirName().isEmpty()) {
			p.drawText(_textRect.adjusted(5, 0, 0, 0), Qt::AlignLeft | Qt::AlignVCenter, dir.dirName());
		}
	}
}
