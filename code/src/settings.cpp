#include "settings.h"

#include "library/librarytreeview.h"
#include "library/libraryfilterlineedit.h"
#include "mainwindow.h"
#include "playlist.h"
#include "tabplaylist.h"

#include <QHeaderView>
#include <QScrollBar>
#include <QTabWidget>

#include <QtDebug>

Settings* Settings::settings = NULL;

/** Private constructor. */
Settings::Settings(const QString &organization, const QString &application)
	: QSettings(organization, application)
{
	QStringList filenames;
	filenames << ":/stylesheets/playlist";
	filenames << ":/stylesheets/librarytreeview";
	filenames << ":/stylesheets/tageditor";
	filenames << ":/stylesheets/qscrollbar";
	filenames << ":/stylesheets/qslider";

	foreach (QString filename, filenames) {
		QFile f(filename);
		f.open(QFile::ReadOnly);
		stylesheets.insert(filename, f.readAll());
		f.close();
	}
}

/** Singleton pattern to be able to easily use settings everywhere in the app. */
Settings* Settings::getInstance()
{
	if (settings == NULL) {
		settings = new Settings;
	}
	return settings;
}

/** Returns the actual theme name. */
QString Settings::theme() const
{
	QString theme = value("theme").toString();
	if (theme.isEmpty()) {
		return "oxygen";
	} else {
		return theme;
	}
}

/** Return the actual size of media buttons. */
int Settings::buttonsSize() const
{
	int s = value("buttonsSize").toInt();
	if (s == 0) {
		return 36;
	} else {
		return s;
	}
}

bool Settings::buttonsFlat() const
{
	QVariant ok = value("buttonsFlat");
	if (ok.isValid()) {
		return ok.toBool();
	} else {
		return true;
	}
}

/** Returns true if the button in parameter is visible or not. */
bool Settings::isVisible(MediaButton *b) const
{
   QVariant ok = value(b->objectName());
   if (ok.isValid()) {
	   return ok.toBool();
   } else {
	   // For the first run, show buttons anyway
	   return (true && b->objectName() != "pauseButton");
   }
}

/** Returns the language of the application. */
QString Settings::language()
{
	QString l = value("language").toString();
	if (l.isEmpty()) {
		l = QLocale::system().uiLanguages().first().left(2);
		setValue("language", l);
		return l;
	} else {
		return l;
	}
}

/** Returns the font of the application. */
QFont Settings::font(const FontFamily fontFamily)
{
	QFont font;
	switch(fontFamily) {
	case PLAYLIST:
	case LIBRARY:
	case MENUS:
		QVariant vFont = value(QString(fontFamily));
		if (!vFont.isNull()) {
			font = QFont(vFont.toString());
		}
		font.setPointSize(this->fontSize(fontFamily));
	}
	return font;
}

/** Sets the font of the application. */
int Settings::fontSize(const FontFamily fontFamily)
{
	fontPointSizeMap = this->value("fontPointSizeMap").toMap();
	int pointSize = fontPointSizeMap.value(QString(fontFamily)).toInt();
	if (pointSize == 0) {
		pointSize = 12;
	}
	return pointSize;
}

bool Settings::toggleSeparators() const
{
	if (value("alphabeticalSeparators").isNull()) {
		return true;
	} else {
		return value("alphabeticalSeparators").toBool();
	}
}

/** Adds a new path in the application. */
void Settings::addMusicLocation(const QString &location)
{
	locations = value("musicLocations").toList();
	if (!locations.contains(location)) {
		locations.append(QVariant(location));
		setValue("musicLocations", locations);
	}
}

/** Removes a path in the application. */
void Settings::removeMusicLocation(const QString &location)
{
	locations = value("musicLocations").toList();
	locations.removeOne(location);
	setValue("musicLocations", locations);
	if (locations.isEmpty()) {
		remove("musicLocations");
	}
}

bool Settings::withCovers() const
{
	if (value("covers").isNull()) {
		return true;
	} else {
		return value("covers").toBool();
	}
}

/** Returns the size of a cover. */
int Settings::coverSize() const
{
	int size = value("coverSize").toInt();
	if (size == 0) {
		size = 48;
	}
	return size;
}

/** Returns the size of the buffer for a cover. */
int Settings::bufferedCoverSize() const
{
	int buffer = value("bufferedCoverSize").toInt();
	if (buffer == 0) {
		buffer = 128;
	}
	return buffer;
}

void Settings::setShortcut(const QString &objectName, int keySequence)
{
	QMap<QString, QVariant> shortcuts = value("shortcuts").toMap();
	if (keySequence == 0) {
		shortcuts.remove(objectName);
	} else {
		shortcuts.insert(objectName, keySequence);
	}
	if (shortcuts.isEmpty()) {
		remove("shortcuts");
	} else {
		setValue("shortcuts", shortcuts);
	}
}

int Settings::shortcut(const QString &objectName) const
{
	QMap<QString, QVariant> shortcuts = value("shortcuts").toMap();
	return shortcuts.value(objectName).toInt();
}

QMap<QString, QVariant> Settings::shortcuts() const
{
	return value("shortcuts").toMap();
}

/** Sets if the button in parameter is visible or not. */
void Settings::setVisible(MediaButton *b, const bool &value)
{
	setValue(b->objectName(), value);
	// The only buttons which are checkable are repeat and shuffle buttons
	if (b->isCheckable() && !value) {
		/// FIXME
		setRepeatPlayBack(value);
	}
}

bool Settings::hasCustomIcon(MediaButton *b) const
{
	QMap<QString, QVariant> customIcons = value("customIcons").toMap();
	return customIcons.value(b->objectName()).toBool();
}

void Settings::setCustomIcon(MediaButton *b, const QString &iconPath)
{
	QMap<QString, QVariant> customIcons = value("customIcons").toMap();
	if (iconPath.isEmpty()) {
		customIcons.remove(b->objectName());
	} else {
		customIcons.insert(b->objectName(), iconPath);
	}
	setValue("customIcons", customIcons);
}

const QString Settings::customIcon(MediaButton *b, bool toggled) const
{
	QMap<QString, QVariant> customIcons = value("customIcons").toMap();
	if (toggled) {
		return customIcons.value("pauseButton").toString();
	} else {
		return customIcons.value(b->objectName()).toString();
	}
}

/// PlayBack options
qint64 Settings::playbackSeekTime() const
{
	qint64 t = value("playbackSeekTime").toLongLong();
	if (t == 0) {
		return 5000;
	} else {
		return t;
	}
}

bool Settings::playbackKeepPlaylists() const
{
	QVariant b = value("playbackKeepPlaylists");
	if (b.isValid()) {
		return b.toBool();
	} else {
		return true;
	}
}

/// Colors
bool Settings::colorsAlternateBG() const
{
	QVariant b = value("colorsAlternateBG");
	if (b.isValid()) {
		return b.toBool();
	} else {
		return true;
	}
}

bool Settings::customColors() const
{
	QVariant b = value("customColors");
	if (b.isValid()) {
		return b.toBool();
	} else {
		return true;
	}
}

/** Should move in specific files in .qrc */
QString Settings::styleSheet(QWidget *w) const
{
	QString styleSheet;
	QMap<QString, QVariant> map = value("styleSheet").toMap();
	if (qobject_cast<Playlist*>(w) != NULL) {

		styleSheet = map.value(Playlist::staticMetaObject.className()).toString();
		if (styleSheet.isEmpty()) {
			styleSheet = stylesheets[":/stylesheets/playlist"];
		}

	} else if (qobject_cast<TagEditorTableWidget*>(w) != NULL) {

		styleSheet = map.value(TagEditorTableWidget::staticMetaObject.className()).toString();
		if (styleSheet.isEmpty()) {
			styleSheet = stylesheets[":/stylesheets/tageditor"];
		}

	} else if (qobject_cast<LibraryTreeView*>(w) != NULL) {

		styleSheet = map.value(LibraryTreeView::staticMetaObject.className()).toString();
		if (styleSheet.isEmpty()) {
			styleSheet = stylesheets[":/stylesheets/librarytreeview"];
		}

	} else if (qobject_cast<QTreeView*>(w) != NULL) {

		styleSheet = map.value(QTreeView::staticMetaObject.className()).toString();
		if (styleSheet.isEmpty()) {
			styleSheet = "QTreeView { padding: 1px; border: 1px solid grey; border-top: 0px; color: #000000; background-color: #ffffff; }";
		}

	} else if (qobject_cast<LibraryFilterLineEdit*>(w) != NULL) {

		styleSheet = map.value(LibraryFilterLineEdit::staticMetaObject.className()).toString();
		if (styleSheet.isEmpty()) {
			styleSheet = "LibraryFilterLineEdit { background-color: #ffffff; border: 1px solid #d5d5d5; border-radius: 10px; padding-left: 6px; padding-right: %1px; }";
		}

	} else if (qobject_cast<QTabWidget*>(w) != NULL) {

		styleSheet = map.value(QTabWidget::staticMetaObject.className()).toString();
		if (styleSheet.isEmpty()) {
			// The first selected tab has nothing to overlap with on the left
			styleSheet = "::tab:first:selected { margin-left: 0; } ";
			styleSheet += "::tab { padding-left: 10px; padding-right: 10px; padding-bottom: 2px; }";
			styleSheet += "::tab:first:!selected { margin-left: 2px; }";
			styleSheet += "::tab:!selected { padding-bottom: 0px; margin-top: 2px; border: 1px solid grey; ";
			styleSheet += "background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #f2f2f2, stop:1 #cfcfcf); }";
			styleSheet += "::tab:selected { padding-top: 4px; border-top-left-radius: 6px; border-top-right-radius: 6px; margin-left: -2px; margin-right: -2px; ";
			styleSheet += "border: 1px solid grey; border-bottom: 0px; color: #000000; background-color: #ffffff; }";
		}
	} else if (qobject_cast<QHeaderView*>(w) != NULL) {

		styleSheet = map.value(QHeaderView::staticMetaObject.className()).toString();
		if (styleSheet.isEmpty()) {
			styleSheet = "QHeaderView::section { padding-top: 4px; padding-bottom: 4px; border: 0px; border-bottom: 1px solid #d5d5d5; color: #000000; ";
			styleSheet += "background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ffffff, stop:1 #f0f0f0); }";
		}

	} else if (qobject_cast<MainWindow*>(w) != NULL) {

		styleSheet = map.value(MainWindow::staticMetaObject.className()).toString();
		if (styleSheet.isEmpty()) {
			styleSheet = "MainWindow { background-color: #f0f0f0; }";
		}

	} else if (qobject_cast<QScrollBar*>(w) != NULL) {

		styleSheet = map.value(QScrollBar::staticMetaObject.className()).toString();
		if (styleSheet.isEmpty()) {
			styleSheet = stylesheets[":/stylesheets/qscrollbar"];
		}

	} else if (qobject_cast<QSplitter*>(w) != NULL) {

		styleSheet = map.value(QSplitter::staticMetaObject.className()).toString();
		if (styleSheet.isEmpty()) {
			styleSheet = "QSplitter { width: 3px; background-color: #f0f0f0; } ";
			styleSheet += "QSplitter::handle { background-color: #f0f0f0; ";
			styleSheet += "padding-top: 20px; padding-bottom: 20px; ";
			styleSheet += "border: 0px; } ";
		}

	} else if (qobject_cast<MediaButton*>(w) != NULL) {

		/*styleSheet = map.value(MediaButton::staticMetaObject.className()).toString();
		if (styleSheet.isEmpty()) {
			styleSheet = "MediaButton:off { background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ff0000, stop:1 #00ff00); } ";
		}*/

	/*} else if (qobject_cast<SeekSlider*>(w) != NULL) {

		styleSheet = map.value(SeekSlider::staticMetaObject.className()).toString();
		if (styleSheet.isEmpty()) {
			styleSheet = stylesheets[":/stylesheets/qslider"];
		}

	} else if (qobject_cast<VolumeSlider*>(w) != NULL) {

		styleSheet = map.value(VolumeSlider::staticMetaObject.className()).toString();
		if (styleSheet.isEmpty()) {
			styleSheet = stylesheets[":/stylesheets/qslider"];
		}

	*/
	} else if (qobject_cast<QSlider*>(w) != NULL) {

		styleSheet = map.value(QSlider::staticMetaObject.className()).toString();
		if (styleSheet.isEmpty()) {
			styleSheet = stylesheets[":/stylesheets/qslider"];
		}

	} else if (w == NULL) {

		styleSheet = map.value(QWidget::staticMetaObject.className()).toString();
		if (styleSheet.isEmpty()) {
			styleSheet = "QWidget { border-left: 1px solid grey; border-right: 1px solid grey; background-color: #ffffff; }";
		}

	} else {

		styleSheet = map.value(w->metaObject()->className()).toString();
	}
	return styleSheet;
}

void Settings::setCustomStyleSheet(QWidget *w)
{
	QMap<QString, QVariant> map = value("styleSheet").toMap();
	map.insert(w->metaObject()->className(), w->styleSheet());
	this->setValue("styleSheet", map);
}
