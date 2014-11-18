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
		// Special root folders like "/" or "D:\" on Windows are empty
		if (d.isRoot()) {
			bool absRoot = true;
			foreach (QFileInfo fileInfo, QDir::drives()) {
				if (fileInfo.absolutePath() == _path.absolutePath()) {
					absRoot = false;
					break;
				}
			}
			if (absRoot) {
				width += 15;
			} else {
				width += fontMetrics().width(_addressBar->getVolumeInfo(d.absolutePath()));
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
	if (SettingsPrivate::instance()->isCustomColors()) {
		g.setColorAt(0, palette.base().color().lighter(110));
		g.setColorAt(1, palette.base().color());
	} else {
		g.setColorAt(0, palette.base().color());
		g.setColorAt(1, palette.window().color());
	}
	p.fillRect(r, g);

	// Compute size of rectangles to display text and right arrow
	QDir dir(_path);
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
	p.setPen(palette.highlight().color());
	if (SettingsPrivate::instance()->isCustomColors()) {
		if (_addressBar->isDown()) {
			p.setBrush(palette.highlight().color().lighter(140));
		} else {
			p.setBrush(palette.highlight().color().lighter());
		}
	} else {
		if (_addressBar->isDown()) {
			p.setBrush(palette.highlight().color().lighter());
		} else {
			p.setBrush(palette.highlight().color().lighter(160));
		}
	}
	if (_highlighted || _textRect.contains(pos)) {
		p.drawRect(_textRect);
	} else if (_highlighted || _arrowRect.contains(pos)) {
		p.drawRect(_textRect);
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
			o.rect = _arrowRect.adjusted(5, 7, -2, -4);
		} else {
			o.rect = _arrowRect.adjusted(2, 7, -5, -4);
		}
		/// XXX: create subclass for root button with special arrow when folders are hidden?
		//p.setBrush(o.palette.highlight());
		p.save();
		p.setPen(Qt::NoPen);
		p.setBrush(o.palette.mid());
		if (_highlighted) {
			p.drawPrimitive(QStyle::PE_IndicatorArrowDown, o);
		} else if (isLeftToRight()) {
			p.drawPrimitive(QStyle::PE_IndicatorArrowRight, o);
		} else {
			p.drawPrimitive(QStyle::PE_IndicatorArrowLeft, o);
		}
		p.restore();
	}

	// Draw folder's name
	QColor lighterBG = palette.highlight().color().lighter();
	QColor highlightedText = palette.highlightedText().color();
	if (qAbs(lighterBG.saturation() - highlightedText.saturation()) > 128 && _highlighted) {
		p.setPen(highlightedText);
	} else {
		p.setPen(palette.windowText().color());
	}

	// Special case for root and drives
	if (dir.isRoot()) {
		bool absRoot = true;
		foreach (QFileInfo fileInfo, QDir::drives()) {
			if (fileInfo.absolutePath() == _path.absolutePath()) {
				// Add a small offset to simulate a pressed button
				if (_highlighted) {
					p.translate(1, 1);
				}
				p.drawText(_textRect.adjusted(5, 0, 0, 0), Qt::AlignLeft | Qt::AlignVCenter,
						   _addressBar->getVolumeInfo(fileInfo.absolutePath()));
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
			// Add a small offset to simulate a pressed button
			if (_highlighted) {
				p.translate(1, 1);
			}
			p.drawText(_textRect.adjusted(5, 0, 0, 0), Qt::AlignLeft | Qt::AlignVCenter, dir.dirName());
		}
	}
}
