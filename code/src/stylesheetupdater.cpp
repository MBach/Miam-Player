#include "stylesheetupdater.h"

#include <QHeaderView>
#include <QWidget>

#include <QtDebug>

#include "playlist.h"
#include "settings.h"
#include "tabplaylist.h"

StyleSheetUpdater::StyleSheetUpdater(QObject *parent) :
	QObject(parent)
{
	regExps.insert("color", QRegExp("( color: )#[0-9a-f]{6}", Qt::CaseInsensitive));
	regExps.insert("background-color", QRegExp("( (selection-)?background-color: )#[0-9a-f]{6}", Qt::CaseInsensitive));
	regExps.insert("linear-gradient", QRegExp("(stop:0 )#[0-9a-f]{6}(, stop:1 )#[0-9a-f]{6}", Qt::CaseInsensitive));
	regExps.insert("alternate-background-color", QRegExp("( alternate-background-color: )#[0-9a-f]{6}", Qt::CaseInsensitive));
	regExps.insert("border-bottom", QRegExp("(border-bottom: 1px solid )#[0-9a-f]{6}", Qt::CaseInsensitive));
}

void StyleSheetUpdater::replace(QWidget *target, const QString &key, const QColor &color)
{
	QString styleSheet = target->styleSheet();
	if (key == "color") {

		if (styleSheet.contains(regExps[key])) {
			styleSheet.replace(regExps[key], "\\1" + color.name());
		}

	} else if (key == "background-color") {

		if (qobject_cast<QHeaderView*>(target) != NULL) {
			QPair<QColor, QColor> newColors = this->makeAlternative(color);
			styleSheet.replace(regExps["linear-gradient"], "\\1" + color.name() + "\\2" + newColors.first.name());
			styleSheet.replace(regExps["border-bottom"], "\\1" + newColors.second.name());
		} else if (qobject_cast<TabPlaylist*>(target) != NULL) {
			QPair<QColor, QColor> newColors = this->makeAlternative(color);
			styleSheet.replace(regExps["linear-gradient"], "\\1" + color.name() + "\\2" + newColors.second.name());
			styleSheet.replace(regExps[key], "\\1" + color.name());
		} else{
			styleSheet.replace(regExps[key], "\\1" + color.name());
		}

	} else if (key == "selection-background-color") {
		/// TODO because there's a small bug when using the same ExpReg for background-color
		/// If the selected item is on an alternate cell then we're currently not displaying the alternate color
		/// but just the background-color
	} else if (key == "alternate-background-color" && qobject_cast<Playlist*>(target) != NULL) {

		Playlist *p = qobject_cast<Playlist*>(target);
		if (!p->styleSheet().contains(regExps[key])) {
			styleSheet.insert(10, ' ' + key + ": " + color.name() + ';');
		} else {
			styleSheet.replace(regExps[key], "\\1" + color.name());
		}
	}
	target->setStyleSheet(styleSheet);

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
