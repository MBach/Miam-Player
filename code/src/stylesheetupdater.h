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
	QMap<QString, QRegExp> regExps;

public:
	StyleSheetUpdater(QObject *parent = 0);

	void replace(QWidget *target, const QString &key, const QColor &color);

	QPair<QColor, QColor> makeAlternative(const QColor &color);
};

#endif // STYLESHEETUPDATER_H
