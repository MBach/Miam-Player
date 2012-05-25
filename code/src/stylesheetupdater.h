#ifndef STYLESHEETUPDATER_H
#define STYLESHEETUPDATER_H

#include <QColor>
#include <QMap>
#include <QObject>
#include <QPair>

class StyleSheetUpdater : public QObject
{
	Q_OBJECT
private:
	Q_ENUMS(Element)

	QMap<int, QRegExp> regExps;

public:
	enum Element { BACKGROUND,
				   GLOBAL_BACKGROUND,
				   TEXT,
				   LINEAR_GRADIENT,
				   ALTERNATE_BACKGROUND,
				   BORDER_BOTTOM
				 };

	StyleSheetUpdater(QObject *parent = 0);

	QPair<QColor, QColor> makeAlternative(const QColor &color);

	void replace(QList<QWidget*> targets, Element key, const QColor &color);

private:
	/** Dispatch instances and get their correct stylesheet. */
	void replace(QWidget *target, Element key, const QColor &color);
};

#endif // STYLESHEETUPDATER_H
