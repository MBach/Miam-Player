#ifndef REFLECTOR_H
#define REFLECTOR_H

#include <QWidget>

class Reflector : public QWidget
{
	Q_OBJECT
private:
	QList<QWidget *> targets;

public:
	Reflector(QWidget *parent = 0);

	inline void addInstance(QWidget *w) { targets.append(w); }

	inline QList<QWidget *> associatedInstances() { return targets; }

};

#endif // REFLECTOR_H
