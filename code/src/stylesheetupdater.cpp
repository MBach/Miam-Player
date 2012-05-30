#include "stylesheetupdater.h"

#include <QHeaderView>
#include <QScrollBar>
#include <QWidget>

#include <QtDebug>

#include "librarytreeview.h"
#include "playlist.h"
#include "settings.h"
#include "tabplaylist.h"

#include "reflector.h"

StyleSheetUpdater::StyleSheetUpdater(QObject *parent) :
	QObject(parent)
{
	regExps.insert(TEXT, QRegExp("( color: )#[0-9a-f]{6}", Qt::CaseInsensitive));
	regExps.insert(BACKGROUND, QRegExp("( (selection-)?background(-color)?: )#[0-9a-f]{6}", Qt::CaseInsensitive));
	regExps.insert(SIMPLE_LINEAR_GRADIENT, QRegExp("(stop:0 )#[0-9a-f]{6}(, stop:1 )#[0-9a-f]{6}", Qt::CaseInsensitive));
	regExps.insert(COMPLEX_LINEAR_GRADIENT, QRegExp("(stop:0 )#[0-9a-f]{6}(, stop:0.4 )#[0-9a-f]{6}(, stop:0.5 )#[0-9a-f]{6}(, stop:1 )#[0-9a-f]{6}", Qt::CaseInsensitive));
	regExps.insert(ALTERNATE_BACKGROUND, QRegExp("( alternate-background-color: )#[0-9a-f]{6}", Qt::CaseInsensitive));
	regExps.insert(BORDER, QRegExp("(border: 1px solid )#[0-9a-f]{6}", Qt::CaseInsensitive));
	regExps.insert(BORDER_BOTTOM, QRegExp("(border-bottom: 1px solid )#[0-9a-f]{6}", Qt::CaseInsensitive));
}

void StyleSheetUpdater::replace(Reflector *reflector, const QColor &color)
{
	foreach (QWidget *target, reflector->associatedInstances()) {
		this->replace(target, reflector->key(), color);
	}

	// Finally, replaces the sender itself
	this->replace(reflector, BACKGROUND, color);
	reflector->setColor(color);
}

/** Create a complex linear gradient with 4 colors. */
QList<QColor> StyleSheetUpdater::makeComplexLinearGradient(const QColor &color)
{
	QColor hsvColor = color.toHsv();
	QList<QColor> gradient;
	const int offset = 16;
	if (hsvColor.value() > offset) {
		gradient.append(color);
		gradient.append(color.darker(110));
		gradient.append(color.darker(120));
		gradient.append(color.darker(130));
	} else {
		gradient.append(color);
		gradient.append(color.lighter(110));
		gradient.append(color.lighter(120));
		gradient.append(color.lighter(130));
	}
	return gradient;
}

/** Create a simple linear gradient with 2 colors. */
QList<QColor> StyleSheetUpdater::makeSimpleLinearGradient(const QColor &color)
{
	QColor hsvColor = color.toHsv();
	QList<QColor> gradient;
	const int offset = 16;
	if (hsvColor.value() > offset) {
		gradient.append(color);
		gradient.append(color.darker(125));

	} else {
		gradient.append(color);
		gradient.append(color.lighter(125));
	}
	return gradient;
}

/*QColor StyleSheetUpdater::makeLighter(const QColor &color)
{
	QColor lighter = color.toHsv();
	const int offset = 16;
	const int minValue = 16;
	if (lighter.saturation() - offset > minValue) {
		lighter.setHsl(lighter.hue(), lighter.saturation() - offset, lighter.value());
	} else {
		lighter.setHsl(lighter.hue(), minValue, lighter.value());
	}
	return lighter;
}*/

/** Dispatch instances and get their correct stylesheet. */
void StyleSheetUpdater::replace(QWidget *target, Element key, const QColor &color)
{
	QString styleSheet = target->styleSheet();
	switch(key) {
	case TEXT:
		if (styleSheet.contains(regExps[key])) {
			styleSheet.replace(regExps[key], "\\1" + color.name());
		}
		break;
	case BACKGROUND:
		if (qobject_cast<QHeaderView*>(target) != NULL) {

			QList<QColor> grad = this->makeSimpleLinearGradient(color);
			styleSheet.replace(regExps[SIMPLE_LINEAR_GRADIENT], "\\1" + grad.at(0).name() + "\\2" + grad.at(1).name());
			styleSheet.replace(regExps[BORDER_BOTTOM], "\\1" + grad.at(1).darker(125).name());

		} else if (qobject_cast<TabPlaylist*>(target) != NULL) {

			QList<QColor> grad = this->makeComplexLinearGradient(color);
			styleSheet.replace(regExps[COMPLEX_LINEAR_GRADIENT], "\\1" + grad.at(0).name() + "\\2" + grad.at(1).name() + "\\3" + grad.at(2).name() + "\\4" + grad.at(3).name());
			styleSheet.replace(regExps[key], "\\1" + color.name());

		} else if (qobject_cast<Playlist*>(target) != NULL) {

			Playlist *p = qobject_cast<Playlist*>(target);
			styleSheet.replace(regExps[key], "\\1" + color.name());
			if (p->alternatingRowColors()) {
				QList<QColor> grad = this->makeComplexLinearGradient(color);
				styleSheet.replace(regExps[ALTERNATE_BACKGROUND], "\\1" + grad.at(0).name());
			}
		} else if (qobject_cast<QScrollBar*>(target) != NULL) {

			// In a complex stylesheet, there are multiples occurrences of qlineargradient.
			// So it's not possible to directly search & replace the pattern without creating a substring first!

			// Handle
			int l = styleSheet.indexOf("QScrollBar::handle:vertical {");
			int r = styleSheet.indexOf('}', l);
			QString substring = styleSheet.mid(l, r - l);
			QList<QColor> grad = this->makeComplexLinearGradient(color);
			substring.replace(regExps[COMPLEX_LINEAR_GRADIENT], "\\1" + grad.at(0).name() + "\\2" + grad.at(1).name() + "\\3" + grad.at(2).name() + "\\4" + grad.at(3).name());
			substring.replace(regExps[BORDER], "\\1" + grad.at(3).darker(125).name());
			styleSheet = styleSheet.left(l) + substring + styleSheet.mid(r);

			// Borders
			l = styleSheet.indexOf("QScrollBar:vertical {");
			r = styleSheet.indexOf('}', l);
			substring = styleSheet.mid(l, r - l);
			substring.replace(regExps[BORDER], "\\1" + color.darker(110).name());
			styleSheet = styleSheet.left(l) + substring + styleSheet.mid(r);

			l = styleSheet.indexOf("QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {");
			r = styleSheet.indexOf('}', l);
			substring = styleSheet.mid(l, r - l);
			substring.replace(regExps[BORDER], "\\1" + color.darker(110).name());
			substring.replace(regExps[key], "\\1" + color.lighter(110).name());
			styleSheet = styleSheet.left(l) + substring + styleSheet.mid(r);

			// Background
			l = styleSheet.indexOf("QScrollBar::add-page, QScrollBar::sub-page {");
			r = styleSheet.indexOf('}', l);
			substring = styleSheet.mid(l, r - l);
			substring.replace(regExps[key], "\\1" + color.lighter(110).name());
			styleSheet = styleSheet.left(l) + substring + styleSheet.mid(r);

		} else {
			styleSheet.replace(regExps[key], "\\1" + color.name());
		}
		break;
	//case SELECTION_BACKGROUND:
		/// TODO because there's a small bug when using the same ExpReg for background-color
		/// If the selected item is on an alternate cell then we're currently not displaying the alternate color
		/// but just the background-color
	//	break;
	case GLOBAL_BACKGROUND:
		styleSheet.replace(regExps[BACKGROUND], "\\1" + color.name());
		break;
	default:
		/// TODO?
		break;
	}
	target->setStyleSheet(styleSheet);
}
