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
			bool absRoot = true;
			for (QFileInfo fileInfo : QDir::drives()) {
				if (fileInfo.absolutePath() == _path.absolutePath()) {
					absRoot = false;
					break;
				}
			}
			if (_isAbsoluteRoot) {
				width += 20;
			} else {
				width += qMax(20, fontMetrics().width(AddressBar::getVolumeInfo(d.absolutePath())));
			}
		}
		width += 28;
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
		_addressBar->init(_path);
	}
}

/** Redefined. */
void AddressBarButton::paintEvent(QPaintEvent *)
{
	QStylePainter p(this);
	QRect r = rect().adjusted(0, 1, -1, -1);
	static const int arrowWidth = 18;

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
		brush = palette.highlight().color().lighter(160);
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
			if (_textRect.contains(pos)) {
				p.setPen(palette.highlight().color());
				p.setBrush(brush);
				p.drawRect(_textRect);
				p.drawRect(_arrowRect);
			} else if (_arrowRect.contains(pos)) {
				p.setPen(palette.mid().color());
				p.setBrush(palette.midlight());
				p.drawRect(_textRect);
				p.setPen(palette.highlight().color());
				p.setBrush(brush);
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
			p.drawText(_textRect.adjusted(5, 0, 0, 0), Qt::AlignCenter, _path.dirName());
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
			if (isLeftToRight()) {
				o.rect = _arrowRect.adjusted(5, 7, -2, -4);
			} else {
				o.rect = _arrowRect.adjusted(2, 7, -5, -4);
			}
			if (_highlighted) {
				p.drawPrimitive(QStyle::PE_IndicatorArrowDown, o);
			} else if (isLeftToRight()) {
				p.drawPrimitive(QStyle::PE_IndicatorArrowRight, o);
			} else {
				p.drawPrimitive(QStyle::PE_IndicatorArrowLeft, o);
			}
		}
		p.restore();
	}
}
