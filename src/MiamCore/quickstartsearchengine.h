#ifndef QUICKSTARTSEARCHENGINE_H
#define QUICKSTARTSEARCHENGINE_H

#include <QFileInfo>

#include "miamcore_global.h"

/**
 * \brief		The QuickStartSearchEngine class
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY QuickStartSearchEngine : public QObject
{
	Q_OBJECT
public:
	explicit QuickStartSearchEngine(QObject *parent = 0);

public slots:
	void doSearch();

signals:
	void folderScanned(const QFileInfo &, const int &);
};

#endif // QUICKSTARTSEARCHENGINE_H
