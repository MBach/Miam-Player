#include "addressbarmenu.h"

#include "addressbar.h"
#include <scrollbar.h>

#include <QApplication>
#include <QMouseEvent>
#include <QScrollBar>
#include <QStylePainter>

#include <QtDebug>

AddressBarMenu::AddressBarMenu(AddressBar *addressBar)
	: QListWidget(addressBar)
	, _addressBar(addressBar)
	, _hasSeparator(false)
{
	this->setVerticalScrollBar(new ScrollBar(Qt::Vertical, this));
	this->installEventFilter(this);
	this->setMouseTracking(true);
	this->setUniformItemSizes(false);
	this->setWindowFlags(Qt::Popup);

	connect(this, &QListWidget::itemClicked, [=](QListWidgetItem *item) {
		if (!item->flags().testFlag(Qt::NoItemFlags)) {
			if (item->data(Qt::UserRole).toString() == "\\") {
				qDebug() << "browse network folders";
			} else {
				_addressBar->init(QDir(item->data(Qt::UserRole).toString()));
			}
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
			for (AddressBarButton *b : _addressBar->findChildren<AddressBarButton*>()) {
				b->setHighlighted(false);
			}
			_addressBar->setDown(false);
		}
	}
	return false;
}

void AddressBarMenu::insertSeparator()
{
	QListWidgetItem *s = new QListWidgetItem(this);
	s->setSizeHint(QSize(width(), 9));
	s->setData(Separator, true);
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
	QPalette palette = QApplication::palette();

	int offsetSB = 0;
	if (verticalScrollBar()->isVisible()) {
		offsetSB = verticalScrollBar()->width() - 1;
	}

	// Subdirectories in the popup menu
	for (int i = 0; i < count(); i ++) {
		QListWidgetItem *it = item(i);
		QRect r = this->visualItemRect(it);
		r.setWidth(r.width() - offsetSB);

		if (it->data(Separator).toBool()) {
			p.save();
			p.setPen(palette.midlight().color());
			p.drawLine(r.x(), r.y() + (it->sizeHint().height()) / 2, r.width(), r.y() + (it->sizeHint().height()) / 2);
			p.restore();
			continue;
		}

		r.adjust(1, 1, -4, -1);
		bool isHighlighted = r.contains(mapFromGlobal(QCursor::pos()));
		// Draw: Highlight, Icon, Text
		if (r.isValid()) {
			QRect iconRect(r.x() + 6, r.y() + 2, 19, 19);
			bool itemIsEnabled = true;
			if (it->flags().testFlag(Qt::NoItemFlags)) {
				p.drawPixmap(iconRect, it->icon().pixmap(QSize(19, 19), QIcon::Disabled));
				itemIsEnabled = false;
			} else {
				p.save();
				if (isHighlighted) {
					p.setPen(palette.highlight().color());
					p.setBrush(palette.highlight().color().lighter());
					p.drawRect(r);
				}
				p.restore();
				p.drawPixmap(iconRect, it->icon().pixmap(QSize(19, 19)));
			}

			QRect textRect = r.adjusted(37, 0, 0, 0);
			QString text;
			if (it->text() == "/") {
				text = fontMetrics().elidedText(tr("Computer"), Qt::ElideRight, textRect.width());
			} else {
				text = fontMetrics().elidedText(it->text(), Qt::ElideRight, textRect.width());
			}
			p.save();

			p.setFont(it->font());
			QColor lighterBG = palette.highlight().color().lighter();
			QColor highlightedText = palette.highlightedText().color();
			if (itemIsEnabled && isHighlighted && qAbs(lighterBG.value() - highlightedText.value()) > 128) {
				p.setPen(highlightedText);
			} else {
				if (itemIsEnabled) {
					p.setPen(palette.text().color());
				} else {
					p.setPen(palette.color(QPalette::Disabled, QPalette::Text));
				}
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
		if (_hasSeparator) {
			h = (count() - 1) * sizeHintForRow(0) + 9 + margin;
		} else {
			h = count() * sizeHintForRow(0) + margin;
		}
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
