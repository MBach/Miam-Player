#ifndef MEDIAPLAYLIST_H
#define MEDIAPLAYLIST_H

#include <QMediaPlaylist>

#include "miamcore_global.h"

class MIAMCORE_LIBRARY MediaPlaylist : public QMediaPlaylist
{
	Q_OBJECT
private:
	std::vector<int> _randomIndexes;
	int _idx;

public:
	explicit MediaPlaylist(QObject *parent = NULL);

	void shuffle();

public slots:
	void next();

private:
	void createRandom();

	void resetRandom();
};

#endif // MEDIAPLAYLIST_H
