#ifndef REFLECTOR_H
#define REFLECTOR_H

#include <QWidget>

#include "dialogs/stylesheetupdater.h"

class StyleSheetUpdater;

/**
 * Reflector Class is only designed to help the way one can customize colors.
 * It keeps a list of objects that can be repainted. Those instances are grouped by category.
 */
class Reflector : public QWidget
{
	Q_OBJECT
private:
	/** Group of paintables elements (in a specific category). */
	QList<QWidget *> targets;

	/** The category of paintables elements. */
	StyleSheetUpdater::Element category;

	/** The color used in the preview pane in the Customize Theme Dialog. */
	QColor backgroundColor;

	StyleSheetUpdater *styleSheetUpdater;

public:
	Reflector(QWidget *parent = 0);

	/** Link a repaintable element to this class. */
	void addInstance(QWidget *w, bool append = true);

	/** Link repaintables elements to this class. */
	void addInstances(QList<QWidget *> list);

	/** Getter to the group of paintables elements. */
	inline QList<QWidget *> associatedInstances() {	return targets;	}

	/** Setter to the category of paintables elements. */
	inline void setStyleSheetUpdater(StyleSheetUpdater *s, StyleSheetUpdater::Element key)
	{
		this->styleSheetUpdater = s;
		this->category = key;
	}

	/** Getter to the category of paintables elements. */
	inline StyleSheetUpdater::Element key() { return category; }

	/** Getter to the color used in the preview pane in the Customize Theme Dialog. */
	inline QColor color() const { return this->backgroundColor; }

	/** Setter to the color used in the preview pane in the Customize Theme Dialog. */
	inline void setColor(const QColor &color) { this->backgroundColor = color; }

protected:
	/** Redefined to be able to reflect the color of the elements in the Customize Theme Dialog. */
	void paintEvent(QPaintEvent *);

};

#endif // REFLECTOR_H
