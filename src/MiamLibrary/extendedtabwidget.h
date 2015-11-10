#ifndef EXTENDEDTAB_H
#define EXTENDEDTAB_H

#include <QAction>
#include <QTabWidget>

#include "extendedtabbar.h"
#include "miamlibrary_global.hpp"

/**
 * \brief		The ExtendedTabWidget class does only one thing: bind shortcuts for tab Library and tab FileSystem.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMLIBRARY_LIBRARY ExtendedTabWidget : public QTabWidget
{
	Q_OBJECT
public:
	explicit ExtendedTabWidget(QWidget *parent = nullptr) : QTabWidget(parent) { setTabBar(new ExtendedTabBar(this)); }

	inline void setShortcut(const QString &objectName, const QKeySequence &sequence) {
		int i = 0;
		if (objectName == "showTabFilesystem") {
			i = 1;
		}
		QAction *shortcutAction = new QAction(this);
		connect(shortcutAction, &QAction::triggered, this, [=]() { setCurrentIndex(i); });
		shortcutAction->setShortcut(sequence);
		this->addAction(shortcutAction);
	}
};

#endif // EXTENDEDTAB_H
