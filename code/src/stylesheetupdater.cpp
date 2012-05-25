#include "stylesheetupdater.h"

#include <QHeaderView>
#include <QWidget>

#include <QtDebug>

#include "librarytreeview.h"
#include "playlist.h"
#include "settings.h"
#include "tabplaylist.h"

StyleSheetUpdater::StyleSheetUpdater(QObject *parent) :
	QObject(parent)
{
	regExps.insert(TEXT, QRegExp("( color: )#[0-9a-f]{6}", Qt::CaseInsensitive));
	regExps.insert(BACKGROUND, QRegExp("( (selection-)?background(-color)?: )#[0-9a-f]{6}", Qt::CaseInsensitive));
	regExps.insert(LINEAR_GRADIENT, QRegExp("(stop:0 )#[0-9a-f]{6}(, stop:1 )#[0-9a-f]{6}", Qt::CaseInsensitive));
	regExps.insert(ALTERNATE_BACKGROUND, QRegExp("( alternate-background-color: )#[0-9a-f]{6}", Qt::CaseInsensitive));
	regExps.insert(BORDER_BOTTOM, QRegExp("(border-bottom: 1px solid )#[0-9a-f]{6}", Qt::CaseInsensitive));
}

QPair<QColor, QColor> StyleSheetUpdater::makeAlternative(const QColor &color)
{
	QColor hsvColor = color.toHsv();
	QColor alternateColor;
	QColor borderColor;
	const int offset = 16;
	if (hsvColor.value() > offset) {
		alternateColor.setHsv(hsvColor.hue(), hsvColor.saturation(), hsvColor.value() - offset);
		borderColor = alternateColor.darker(150);
	} else {
		alternateColor.setHsv(hsvColor.hue(), hsvColor.saturation(), hsvColor.value() + offset);
		borderColor = alternateColor.lighter();
	}
	return qMakePair(alternateColor, borderColor);
}

void StyleSheetUpdater::replace(QList<QWidget *> targets, Element key, const QColor &color)
{
	foreach (QWidget *target, targets) {
		this->replace(target, key, color);
	}
}

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

			QPair<QColor, QColor> newColors = this->makeAlternative(color);
			styleSheet.replace(regExps[LINEAR_GRADIENT], "\\1" + color.name() + "\\2" + newColors.first.name());
			styleSheet.replace(regExps[BORDER_BOTTOM], "\\1" + newColors.second.name());

		} else if (qobject_cast<TabPlaylist*>(target) != NULL) {

			QPair<QColor, QColor> newColors = this->makeAlternative(color);
			styleSheet.replace(regExps[LINEAR_GRADIENT], "\\1" + color.name() + "\\2" + newColors.second.name());
			styleSheet.replace(regExps[key], "\\1" + color.name());

		} else if (qobject_cast<Playlist*>(target) != NULL) {

			Playlist *p = qobject_cast<Playlist*>(target);
			styleSheet.replace(regExps[key], "\\1" + color.name());
			if (p->alternatingRowColors()) {
				QPair<QColor, QColor> newColors = this->makeAlternative(color);
				styleSheet.replace(regExps[ALTERNATE_BACKGROUND], "\\1" + newColors.first.name());
			}
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
		/// TODO
		break;
	}
	target->setStyleSheet(styleSheet);
}
