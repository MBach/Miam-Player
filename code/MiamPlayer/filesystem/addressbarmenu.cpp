#include "addressbarmenu.h"

#include "addressbar.h"

#include <QApplication>
#include <QMouseEvent>
#include <QScrollBar>
#include <QStylePainter>

#include <QtDebug>

AddressBarMenu::AddressBarMenu(AddressBar *addressBar) :
	QListWidget(addressBar), _addressBar(addressBar), _hasSeparator(false)
{
	this->installEventFilter(this);
	this->setMouseTracking(true);
	this->setUniformItemSizes(true);
	this->setWindowFlags(Qt::Popup);

	connect(this, &QListWidget::itemClicked, [=](QListWidgetItem *item) {
		if (!item->flags().testFlag(Qt::NoItemFlags)) {
			_addressBar->init(QDir(item->data(Qt::UserRole).toString()));
			this->clear();
			this->close();
		}
	});

	this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

bool AddressBarMenu::eventFilter(QObject *, QEvent *event)
{
	if (event->type() == QEvent::MouseMove) {
		QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
		/// XXX: wtf notify() is not working?
		if (!rect().contains(mouseEvent->pos())) {
			_addressBar->findAndHighlightButton(mapToGlobal(mouseEvent->pos()));
		}
	} else if (event->type() == QEvent::MouseButtonPress) {
		// Close this popup when one is clicking outside the menu (otherwise this widget stays on top)
		QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
		if (!rect().contains(mouseEvent->pos())) {
			this->close();
			foreach (AddressBarButton *b, _addressBar->findChildren<AddressBarButton*>()) {
				b->setHighlighted(false);
			}
			_addressBar->setDown(false);
		}
	}
	return false;
}

bool AddressBarMenu::hasSeparator() const
{
	return _hasSeparator;
}

void AddressBarMenu::insertSeparator()
{
	QListWidgetItem *s = new QListWidgetItem(this);
	s->setSizeHint(QSize(width(), 1));
	s->setData(Qt::UserRole + 1, 1);
	_hasSeparator = true;
}

/** Redefined to force update the viewport. */
void AddressBarMenu::mouseMoveEvent(QMouseEvent *e)
{
	this->viewport()->update();
	QListWidget::mouseMoveEvent(e);
}

/** Redefined to be able to display items with the current theme. */
void AddressBarMenu::paintEvent(QPaintEvent *)
{
	QStylePainter p(this->viewport());
	// Vertical frame between icons and text
	p.save();
	p.setPen(QApplication::palette().midlight().color());
	p.drawLine(33, 0, 33, rect().height());
	p.restore();

	int offsetSB = 0;
	if (verticalScrollBar()->isVisible()) {
		offsetSB = verticalScrollBar()->width() - 1;
	}

	// Subdirectories in the popup menu
	for (int i = 0; i < count(); i ++) {
		QListWidgetItem *it = item(i);
		QRect r = this->visualItemRect(it);
		/// FIXME
		//QSize s = it->sizeHint();
		//QRect r(0, i * s.height(), );
		//qDebug() << "r" << r;
		r.setWidth(r.width() - offsetSB);

		if (it->data(Qt::UserRole + 1).toInt() == 1) {
			p.save();
			p.setPen(QApplication::palette().midlight().color());
			p.drawLine(r.x(), r.y(), r.width(), r.y());
			p.restore();
			continue;
		}
		r.adjust(1, 1, -4, -1);
		// Draw: Highlight, Icon, Text
		if (r.isValid()) {
			QRect iconRect(r.x() + 6, r.y() + 2, 19, 19);
			bool itemIsEnabled = true;
			bool isHighLighted = false;
			if (it->flags().testFlag(Qt::NoItemFlags)) {
				p.drawPixmap(iconRect, it->icon().pixmap(QSize(19, 19), QIcon::Disabled));
				itemIsEnabled = false;
			} else {
				p.save();
				if (r.contains(mapFromGlobal(QCursor::pos()))) {
					p.setPen(QApplication::palette().highlight().color());
					p.setBrush(QApplication::palette().highlight().color().lighter());
					p.drawRect(r);
					p.setPen(QColor(192, 192, 192, 128));
					p.drawLine(33, r.top() + 1, 33, r.bottom());
					isHighLighted = true;
				}
				p.restore();
				p.drawPixmap(iconRect, it->icon().pixmap(QSize(19, 19)));
			}

			QRect textRect = r.adjusted(37, 0, 0, 0);
			QString text = fontMetrics().elidedText(it->text(), Qt::ElideRight, textRect.width());
			p.save();
			if (!itemIsEnabled) {
				p.setPen(QApplication::palette().color(QPalette::Disabled, QPalette::WindowText));
			}
			p.setFont(it->font());
			QColor lighterBG = QApplication::palette().highlight().color().lighter();
			QColor highlightedText = QApplication::palette().highlightedText().color();
			if (qAbs(lighterBG.saturation() - highlightedText.saturation()) > 128) {
				p.setPen(highlightedText);
			} else {
				p.setPen(QApplication::palette().windowText().color());
			}

			p.drawText(textRect, text, Qt::AlignLeft | Qt::AlignVCenter);
			p.restore();
		}
	}
}

void AddressBarMenu::moveOrHide(const AddressBarButton *b)
{
	QPoint globalButtonPos;
	if (isLeftToRight()) {
		globalButtonPos = b->mapToGlobal(b->rect().bottomRight());
		globalButtonPos.rx() -= 2 * b->arrowRect().width();
	} else {
		globalButtonPos = b->mapToGlobal(b->rect().bottomLeft());
		globalButtonPos.rx() -= (this->width() - 2 * b->arrowRect().width());
	}
	globalButtonPos.ry() += 1;
	this->move(globalButtonPos);
	this->show();
}

void AddressBarMenu::clear()
{
	_hasSeparator = false;
	QListWidget::clear();
}

void AddressBarMenu::show()
{
	static const int maxBeforeScrolling = 18;
	static const int margin = 4;
	if (count() < maxBeforeScrolling) {
		int h = count() * sizeHintForRow(0) + margin;
		/*if (_hasSeparator) {
			h -= 19; // sizeHintForRow(0) == 22, height(separator) == 3
		}*/
		this->setMinimumHeight(h);
		this->setMaximumHeight(h);
	} else {
		this->setMinimumHeight(maxBeforeScrolling * sizeHintForRow(0) + margin);
		this->setMaximumHeight(maxBeforeScrolling * sizeHintForRow(0) + margin);
	}
	if (count() > 0) {
		QListWidget::show();
	} else {
		QListWidget::hide();
	}
}
