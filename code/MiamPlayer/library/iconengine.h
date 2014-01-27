#ifndef ICONENGINE_H
#define ICONENGINE_H

#include <QIconEngine>
//#include <QObject>

class IconEngine : public QIconEngine
{
public:
	IconEngine() {}

	virtual QIconEngine* clone() const { return NULL; }

	virtual void paint(QPainter * painter, const QRect & rect, QIcon::Mode mode, QIcon::State state);
};

#endif // ICONENGINE_H
