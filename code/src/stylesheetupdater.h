#ifndef STYLESHEETUPDATER_H
#define STYLESHEETUPDATER_H

#include <QColor>
#include <QMap>
#include <QObject>

class Reflector;

class StyleSheetUpdater : public QObject
{
	Q_OBJECT
private:
	Q_ENUMS(Element)

	// QRegExp should be avoided because of their lack of performance
	// User Experience will increased if any changes in UI is faster
	QMap<int, QRegExp> regExps;

public:
	enum Element { BACKGROUND,
				   GLOBAL_BACKGROUND,
				   TEXT,
				   SIMPLE_LINEAR_GRADIENT,
				   COMPLEX_LINEAR_GRADIENT,
				   ALTERNATE_BACKGROUND,
				   BORDER,
				   BORDER_BOTTOM
				 };

	StyleSheetUpdater(QObject *parent = 0);

	void replace(Reflector *reflector, const QColor &color);

	inline const QRegExp regExp(Element key) const { return regExps[key]; }

private:
	/** Create a complex linear gradient with 4 colors. */
	QList<QColor> makeComplexLinearGradient(const QColor &color);

	/** Create a simple linear gradient with 2 colors. */
	QList<QColor> makeSimpleLinearGradient(const QColor &color);

	//QColor makeLighter(const QColor &color);

	/** Dispatch instances and get their correct stylesheet. */
	void replace(QWidget *target, Element key, const QColor &color);
};

#endif // STYLESHEETUPDATER_H
