#ifndef REFLECTOR_H
#define REFLECTOR_H

#include <QWidget>

#include "stylesheetupdater.h"

class Reflector : public QWidget
{
	Q_OBJECT
private:
	QList<QWidget *> targets;

	StyleSheetUpdater::Element k;

public:
	Reflector(QWidget *parent = 0);

	inline void addInstance(QWidget *w) { targets.append(w); }

	inline QList<QWidget *> associatedInstances() { return targets; }

	inline void setKey(StyleSheetUpdater::Element key) { k = key; }

	StyleSheetUpdater::Element key() { return k; }

};

#endif // REFLECTOR_H
