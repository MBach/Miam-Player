#include "reflector.h"

#include <QPainter>
#include <QStyleOption>

#include <QtDebug>

Reflector::Reflector(QWidget *parent) :
	QWidget(parent), backgroundColor(QColor()), styleSheetUpdater(NULL)
{
}

/** Link a repaintable element to this class. */
void Reflector::addInstance(QWidget *w, bool append)
{
	if (append) {
		targets.append(w);
	}

	// When the first element is added, then automatically repaint this reflector in the options
	if (!backgroundColor.isValid() && styleSheetUpdater != NULL) {

		// Find the kind of color which is mutable
		int index = w->styleSheet().indexOf(styleSheetUpdater->regExp(category));
		if (index != -1) {

			// Finally, find the color
			int position = w->styleSheet().indexOf(QRegExp("#[0-9a-f]"), index);
			if (position != -1) {
				backgroundColor = QColor(w->styleSheet().mid(position, 7));
				this->setStyleSheet("Reflector { border: 1px solid #707070; background-color: " + backgroundColor.name() + "; }");
			}
		}
	}
}

/** Link repaintables elements to this class. */
void Reflector::addInstances(QList<QWidget *> list)
{
	targets.append(list);
	foreach (QWidget *w, list) {
		if (w != NULL) {
			this->addInstance(w, false);
		}
	}
}

/** Redefined to be able to reflect the color of the elements in the Customize Theme Dialog. */
void Reflector::paintEvent(QPaintEvent *)
{
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
