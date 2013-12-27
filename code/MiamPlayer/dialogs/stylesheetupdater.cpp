#include "stylesheetupdater.h"

#include <QHeaderView>
#include <QScrollBar>
#include <QWidget>

#include <QtDebug>

#include "settings.h"
#include "../library/librarytreeview.h"
#include "../playlists/playlist.h"
#include "../playlists/tabplaylist.h"
#include "../tageditor/tageditor.h"

#include "reflector.h"

StyleSheetUpdater::StyleSheetUpdater(QObject *parent) :
	QObject(parent)
{
	regExps.insert(TEXT, QRegExp("( color: )#[0-9a-f]{6}", Qt::CaseInsensitive));
	regExps.insert(BACKGROUND, QRegExp("( background(-color)?: )#[0-9a-f]{6}", Qt::CaseInsensitive));
	regExps.insert(SIMPLE_LINEAR_GRADIENT, QRegExp("(stop:0 )#[0-9a-f]{6}(, stop:1 )#[0-9a-f]{6}", Qt::CaseInsensitive));
	regExps.insert(COMPLEX_LINEAR_GRADIENT, QRegExp("(stop:0 )#[0-9a-f]{6}(, stop:0.4 )#[0-9a-f]{6}(, stop:0.5 )#[0-9a-f]{6}(, stop:1 )#[0-9a-f]{6}", Qt::CaseInsensitive));
	regExps.insert(ALTERNATE_BACKGROUND, QRegExp("( alternate-background-color: )#[0-9a-f]{6}", Qt::CaseInsensitive));
	regExps.insert(BORDER, QRegExp("(border: 1px solid )#[0-9a-f]{6}", Qt::CaseInsensitive));
	regExps.insert(BORDER_BOTTOM, QRegExp("(border-bottom: 1px solid )#[0-9a-f]{6}", Qt::CaseInsensitive));
}

/** Create a linear gradient with 2 or 4 colors. */
QList<QColor> StyleSheetUpdater::makeLinearGradient(Element complexity, const QColor &color)
{
	QColor hsvColor = color.toHsv();
	QList<QColor> gradient;
	const int offset = 16;
	if (complexity == SIMPLE_LINEAR_GRADIENT) {
		if (hsvColor.value() > offset) {
			gradient.append(color);
			gradient.append(color.darker(125));

		} else {
			gradient.append(color);
			gradient.append(color.lighter(125));
		}
	} else {
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
	}
	return gradient;
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

			QList<QColor> grad = this->makeLinearGradient(SIMPLE_LINEAR_GRADIENT, color);
			styleSheet.replace(regExps[SIMPLE_LINEAR_GRADIENT], "\\1" + grad.at(0).name() + "\\2" + grad.at(1).name());
			styleSheet.replace(regExps[BORDER_BOTTOM], "\\1" + grad.at(1).darker(125).name());

		} else if (qobject_cast<TabPlaylist*>(target) != NULL) {

			QList<QColor> grad = this->makeLinearGradient(COMPLEX_LINEAR_GRADIENT, color);
			styleSheet.replace(regExps[COMPLEX_LINEAR_GRADIENT], "\\1" + grad.at(0).name() + "\\2" + grad.at(1).name() + "\\3" + grad.at(2).name() + "\\4" + grad.at(3).name());
			styleSheet.replace(regExps[key], "\\1" + color.name());

		} else if (qobject_cast<Playlist*>(target) != NULL) {

			Playlist *p = qobject_cast<Playlist*>(target);
			styleSheet.replace(regExps[key], "\\1" + color.name());
			if (p->alternatingRowColors()) {
				QList<QColor> grad = this->makeLinearGradient(COMPLEX_LINEAR_GRADIENT, color);
				styleSheet.replace(regExps[ALTERNATE_BACKGROUND], "\\1" + grad.at(1).name());
			}

		} else if (qobject_cast<TagEditorTableWidget*>(target) != NULL) {

			QList<QColor> grad = this->makeLinearGradient(COMPLEX_LINEAR_GRADIENT, color);
			styleSheet.replace(regExps[key], "\\1" + color.name());
			styleSheet.replace(regExps[ALTERNATE_BACKGROUND], "\\1" + grad.at(1).name());

		} else if (qobject_cast<QScrollBar*>(target) != NULL) {

			// In a complex stylesheet, there are multiples occurrences of qlineargradient.
			// So it's not possible to directly search & replace the pattern without creating a substring first!

			// Handle
			int l = styleSheet.indexOf("QScrollBar::handle:vertical {");
			int r = styleSheet.indexOf('}', l);
			QString substring = styleSheet.mid(l, r - l);
			QList<QColor> grad = this->makeLinearGradient(COMPLEX_LINEAR_GRADIENT, color);
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

			qDebug() << styleSheet << target->parent()->parent()->objectName();


		} else {
			styleSheet.replace(regExps[key], "\\1" + color.name());
		}
		break;
	case HOVER:
		/// FIXME: too much Copy-paste! Should really be improved with a better design
		/// The main problem is to build dynamic GUI! Is it better to split(;) everything and direct access with KEYS?
		/// How QCSSParser is working?
		if (qobject_cast<Playlist*>(target) != NULL) {

			int l = styleSheet.indexOf("stop:0 rgba(");
			int r = styleSheet.indexOf(';', l);
			QList<QColor> grad = this->makeLinearGradient(SIMPLE_LINEAR_GRADIENT, color);
			QColor f = grad.at(0), s = grad.at(1).darker(125);
			QString substring = "stop:0 rgba(" + QString::number(f.red()) + ',' + QString::number(f.green()) + ',' + QString::number(f.blue()) + ",80), ";
			substring += "stop:1 rgba(" + QString::number(s.red()) + ',' + QString::number(s.green()) + ',' + QString::number(s.blue()) + ",80))";
			styleSheet = styleSheet.left(l) + substring + styleSheet.mid(r);

		} else if (qobject_cast<QScrollBar*>(target) != NULL) {

			int l = styleSheet.indexOf("QScrollBar::add-line:hover:vertical {");
			int r = styleSheet.indexOf('}', l);
			QString substring = styleSheet.mid(l, r - l);
			QList<QColor> grad = this->makeLinearGradient(COMPLEX_LINEAR_GRADIENT, color.lighter(125));
			substring.replace(regExps[COMPLEX_LINEAR_GRADIENT], "\\1" + grad.at(0).name() + "\\2" + grad.at(1).name() + "\\3" + grad.at(2).name() + "\\4" + grad.at(3).name());
			substring.replace(regExps[BORDER], "\\1" + grad.at(3).darker(125).name());
			styleSheet = styleSheet.left(l) + substring + styleSheet.mid(r);

		} else if (qobject_cast<LibraryTreeView*>(target) != NULL) {

			QList<QColor> grad = this->makeLinearGradient(SIMPLE_LINEAR_GRADIENT, color);
			QColor f = grad.at(0), s = grad.at(1);

			// Border (when selected)
			int l = styleSheet.indexOf("LibraryTreeView::item:selected {");
			int r = styleSheet.indexOf('}', l);
			QString substring = styleSheet.mid(l, r - l);
			substring.replace(regExps[BORDER], "\\1" + s.darker(115).name());
			styleSheet = styleSheet.left(l) + substring + styleSheet.mid(r);

			// Background (when selected)
			l = styleSheet.indexOf("stop:0 rgba(");
			r = styleSheet.indexOf(';', l);
			substring = "stop:0 rgba(" + QString::number(f.red()) + ',' + QString::number(f.green()) + ',' + QString::number(f.blue()) + ",80), ";
			substring += "stop:1 rgba(" + QString::number(s.red()) + ',' + QString::number(s.green()) + ',' + QString::number(s.blue()) + ",80))";
			styleSheet = styleSheet.left(l) + substring + styleSheet.mid(r);

			// Border (when hovered)
			l = styleSheet.indexOf("LibraryTreeView::item:hover:!selected {");
			r = styleSheet.indexOf('}', l);
			substring = styleSheet.mid(l, r - l);
			substring.replace(regExps[BORDER], "\\1" + s.lighter(150).name());
			styleSheet = styleSheet.left(l) + substring + styleSheet.mid(r);

			// Background (when hovered)
			l = styleSheet.indexOf("stop:0 rgba(", l);
			r = styleSheet.indexOf(';', l);
			substring = "stop:0 rgba(" + QString::number(f.red()) + ',' + QString::number(f.green()) + ',' + QString::number(f.blue()) + ",50), ";
			substring += "stop:1 rgba(" + QString::number(s.red()) + ',' + QString::number(s.green()) + ',' + QString::number(s.blue()) + ",50))";
			styleSheet = styleSheet.left(l) + substring + styleSheet.mid(r);

		} else if (qobject_cast<MediaButton*>(target) != NULL) {

			/// FIXME
			QList<QColor> grad = this->makeLinearGradient(SIMPLE_LINEAR_GRADIENT, color);

		//} else if (qobject_cast<SeekSlider*>(target) != NULL || qobject_cast<VolumeSlider*>(target) != NULL) {
		} else if (qobject_cast<QSlider*>(target) != NULL) {

			int l = styleSheet.indexOf("QSlider::groove:horizontal {");
			l = styleSheet.indexOf("stop:0 rgba(", l);
			int r = styleSheet.indexOf(';', l);
			QList<QColor> grad = this->makeLinearGradient(SIMPLE_LINEAR_GRADIENT, color);
			QColor f = grad.at(0), s = grad.at(1).darker(125);
			QString substring = "stop:0 rgba(" + QString::number(f.red()) + ',' + QString::number(f.green()) + ',' + QString::number(f.blue()) + ",150), ";
			substring += "stop:1 rgba(" + QString::number(s.red()) + ',' + QString::number(s.green()) + ',' + QString::number(s.blue()) + ",150))";
			styleSheet = styleSheet.left(l) + substring + styleSheet.mid(r);

		}
		break;
	case GLOBAL_BACKGROUND:
		styleSheet.replace(regExps[BACKGROUND], "\\1" + color.name());
		break;
	default:
		/// TODO?
		break;
	}
	target->setStyleSheet(styleSheet);
}

void StyleSheetUpdater::replace(Reflector *reflector, const QColor &color)
{
	foreach (QWidget *target, reflector->associatedInstances()) {
		if (target != NULL) {
			this->replace(target, reflector->key(), color);
		}
	}

	// Finally, replaces the sender itself
	this->replace(reflector, BACKGROUND, color);
	reflector->setColor(color);
}
