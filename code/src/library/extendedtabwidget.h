#ifndef EXTENDEDTAB_H
#define EXTENDEDTAB_H

#include <QTabWidget>

#include "extendedtabbar.h"

class ExtendedTabWidget : public QTabWidget
{
	Q_OBJECT
public:
	explicit ExtendedTabWidget(QWidget *parent = 0) : QTabWidget(parent) { setTabBar(new ExtendedTabBar(this)); }

};

#endif // EXTENDEDTAB_H
