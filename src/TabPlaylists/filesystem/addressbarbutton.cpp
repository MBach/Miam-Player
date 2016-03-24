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

AddressBarButton::AddressBarButton(const QDir &newPath, AddressBar *parent, bool isAbsoluteRoot)
	: QPushButton(parent)
	, _path(newPath)
	, _atLeastOneSubDir(false)
	, _highlighted(false)
	, _isAbsoluteRoot(isAbsoluteRoot)
	, _addressBar(parent)
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
		// Special root folders like "/" or "D:\" on Windows are empty
		if (d.isRoot()) {
			/// XXX?
			for (QFileInfo fileInfo : QDir::drives()) {
				if (fileInfo.absolutePath() == _path.absolutePath()) {
					break;
				}
			}
			if (_isAbsoluteRoot) {
				width += 10;
			} else {
				width += qMax(20, fontMetrics().width(AddressBar::getVolumeInfo(d.absolutePath())));
			}
		}
		width += 28 + this->height() / 2;
		this->setMinimumWidth(width);
	} else {
		this->setMinimumWidth(width + 18);
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
	// We are sure that parent is a QWidget
	qobject_cast<QWidget *>(this->parent())->update();
	if (_addressBar->isDown()) {
		emit aboutToShowMenu();
	}
}

/** Redefined. */
void AddressBarButton::mousePressEvent(QMouseEvent *event)
{
	// mouse press in arrow rect => immediate popup menu
	// in text rect => goto dir, mouse release event is not relevant
	if (_arrowRect.contains(event->pos())) {
		this->setHighlighted(true);
		_addressBar->setDown(true);
		this->update();
	} else if (_textRect.contains(event->pos())) {
		if (_isAbsoluteRoot) {
			emit triggerLineEdit();
			event->accept();
		} else {
			_addressBar->init(_path);
		}
	}
}

/** Redefined. */
void AddressBarButton::paintEvent(QPaintEvent *)
{
	QStylePainter p(this);
	QRect r = rect().adjusted(0, 1, -1, -(1 + extra));
	static const int arrowWidth = r.height();

	QPalette palette = QApplication::palette();
	QLinearGradient g(rect().topLeft(), rect().bottomLeft());
	g.setColorAt(0, palette.base().color());
	g.setColorAt(1, palette.window().color());
	p.fillRect(r, g);

	// Compute size of rectangles to display text and right arrow
	if (_atLeastOneSubDir) {
		if (isLeftToRight()) {
			_arrowRect = QRect(r.width() - arrowWidth, r.y(), arrowWidth, r.height());
			_textRect = QRect(r.x(), r.y(), r.width() - arrowWidth, r.height());
		} else {
			_arrowRect = QRect(r.x(), r.y(), arrowWidth, r.height());
			_textRect = QRect(r.x() + arrowWidth, r.y(), r.width() - arrowWidth, r.height());
		}
	} else {
		_textRect = r.adjusted(0, 0, -5, 0);
	}

	// Highlight button if mouse is over
	QPoint pos = mapFromGlobal(QCursor::pos());
	p.save();
	QBrush brush;
	if (_addressBar->isDown()) {
		brush = palette.highlight().color().lighter();
	} else {
		brush = palette.highlight().color().lighter(lighterValue);
	}

	if (_highlighted) {
		p.setPen(palette.highlight().color());
		p.setBrush(brush);
		p.drawRect(_textRect);
		if (_atLeastOneSubDir) {
			p.drawRect(_arrowRect);
		}
	} else {
		if (_atLeastOneSubDir) {
			if (_textRect.contains(pos) || _arrowRect.contains(pos)) {
				p.setPen(palette.highlight().color());
				p.setBrush(brush);
				p.drawRect(_textRect);
				p.drawRect(_arrowRect);
			} else {
				p.setPen(Qt::NoPen);
				p.setBrush(Qt::NoBrush);
				p.drawRect(_textRect);
				p.drawRect(_arrowRect);
			}
		} else {
			if (_textRect.contains(pos)) {
				p.setPen(palette.highlight().color());
				p.setBrush(brush);
				p.drawRect(_textRect);
			}
		}
	}
	p.restore();

	// Draw folder's name
	QColor lighterBG = palette.highlight().color().lighter();
	QColor highlightedText = palette.highlightedText().color();
	if (qAbs(lighterBG.value() - highlightedText.value()) > 128 && _highlighted) {
		p.setPen(highlightedText);
	} else {
		p.setPen(palette.windowText().color());
	}

	// Special case for root and drives
	bool root = false;
	if (_path.isRoot()) {
		QPixmap pixmap = QFileIconProvider().icon(QFileIconProvider::Computer).pixmap(20, 20);
		QString drive;
		if (_isAbsoluteRoot) {
			pixmap = QFileIconProvider().icon(QFileIconProvider::Computer).pixmap(20, 20);
			if (isLeftToRight()) {
				p.drawPixmap(2, 3, 20, 20, pixmap);
			} else {
				p.drawPixmap(18, 3, 20, 20, pixmap);
			}
		} else {
			drive = AddressBar::getVolumeInfo(_path.absolutePath());
			if (!_isAbsoluteRoot && !drive.isEmpty()) {
				// Add a small offset to simulate a pressed button
				if (_highlighted) {
					p.translate(1, 1);
				}
				p.drawText(_textRect.adjusted(5, 0, 0, 0), Qt::AlignCenter, drive);
			}
			pixmap = QFileIconProvider().icon(QFileIconProvider::Drive).pixmap(20, 20);
		}
	} else {
		if (!_path.dirName().isEmpty()) {
			// Add a small offset to simulate a pressed button
			if (_highlighted) {
				p.translate(1, 1);
			}
			p.drawText(_textRect.adjusted(0, 0, 0, 0), Qt::AlignCenter, _path.dirName());
		}
	}

	if (_atLeastOneSubDir) {
		QStyleOptionButton o;
		o.initFrom(this);
		p.save();
		p.setPen(Qt::NoPen);
		p.setBrush(o.palette.mid());
		if (root && _addressBar->hasHiddenFolders()) {
			/// Right To Left
			QPoint p1(o.rect.x() + 32, o.rect.y() + 11),
					p2(o.rect.x() + 29, o.rect.y() + 14),
					p2b(o.rect.x() + 29, o.rect.y() + 13),
					p3(o.rect.x() + 32, o.rect.y() + 16);
			p.save();
			p.setPen(Qt::black);
			p.setRenderHint(QPainter::Antialiasing);
			p.drawLine(p1, p2);
			p.drawLine(p2b, p3);
			p.translate(4, 0);
			p.drawLine(p1, p2);
			p.drawLine(p2b, p3);
			p.restore();
		} else {
			int w = _arrowRect.width() / 3;
			int h = this->rect().height() / 3;
			QRect indicatorArrow(_arrowRect.x() + w + 1, _arrowRect.y() + h, w, h);

			o.rect = indicatorArrow;
			//qDebug() << Q_FUNC_INFO << o.rect << _arrowRect;
			//p.drawRect(o.rect);

			p.setRenderHint(QPainter::Antialiasing);

			p.save();
			QPen pen(palette.mid().color());
			pen.setWidthF(1.5);
			pen.setJoinStyle(Qt::MiterJoin);
			p.setPen(pen);
			QPolygon pol;
			QPoint p1, p2, p3;
			if (_highlighted) {
				p.translate(0, -1);
				p1 = QPoint(o.rect.x(), o.rect.y() + o.rect.height() / 2);
				p2 = QPoint(o.rect.x() + o.rect.width(), o.rect.y() + o.rect.height() / 2);
				p3 = QPoint(o.rect.x() + o.rect.width() / 2, o.rect.y() + o.rect.height());
			} else if (isLeftToRight()) {
				p1 = QPoint(o.rect.x(), o.rect.y());
				p2 = QPoint(o.rect.x(), o.rect.y() + o.rect.height());
				p3 = QPoint(o.rect.x() + o.rect.width(), o.rect.y() + o.rect.height() / 2);
			} else {
				p1 = QPoint(o.rect.x() + o.rect.width(), o.rect.y());
				p2 = QPoint(o.rect.x() + o.rect.width(), o.rect.y() + o.rect.height());
				p3 = QPoint(o.rect.x(), o.rect.y() + o.rect.height() / 2);
			}
			pol.append(p1);
			pol.append(p2);
			pol.append(p3);
			p.drawPolygon(pol);
			p.restore();
			p.setRenderHint(QPainter::Antialiasing, false);
		}
		p.restore();
	}
}
