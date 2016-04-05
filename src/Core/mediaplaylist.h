#ifndef MEDIAPLAYLIST_H
#define MEDIAPLAYLIST_H

#include <QMediaPlaylist>

#include "miamcore_global.h"

/**
 * \brief		The MediaPlaylist class has been created to have a custom Random mode.
 * \details		Default Random mode doesn't keep in memory which tracks that were played. It can be very confusing to press 'Next'
 *				and to listen the track that just has been played before. Now, it's impossible to have the same track beein played twice
 *				unless all other tracks were played once. Moreover if one skips a track, it's still possible to rewind and play the latter.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY MediaPlaylist : public QMediaPlaylist
{
	Q_OBJECT
private:
	std::vector<int> _randomIndexes;
	int _idx;

public:
	explicit MediaPlaylist(QObject *parent = nullptr);

	void shuffle(int idx);

	void skipBackward();

	void skipForward();

private:
	void createRandom();

	void resetRandom();
};

#endif // MEDIAPLAYLIST_H
