#include "mediaplaylist.h"

#include <algorithm>
#include <ctime>
#include <random>

#include <QtDebug>

MediaPlaylist::MediaPlaylist(QObject *parent)
	: QMediaPlaylist(parent), _idx(0)
{
	connect(this, &QMediaPlaylist::playbackModeChanged, this, [=](PlaybackMode mode) {
		if (mode == Random) {
			this->createRandom();
		} else {
			this->resetRandom();
		}
	});
}

void MediaPlaylist::shuffle()
{
	this->resetRandom();
	this->createRandom();
}

void MediaPlaylist::createRandom()
{
	for (int i = 0; i < mediaCount(); i++) {
		_randomIndexes.push_back(i);
	}

	std::srand(std::time(nullptr));
	std::random_shuffle(_randomIndexes.begin(), _randomIndexes.end());
	_idx = _randomIndexes[0];
}

void MediaPlaylist::resetRandom()
{
	_randomIndexes.clear();
	_idx = 0;
}

void MediaPlaylist::next()
{
	if (playbackMode() == Random) {
		this->setCurrentIndex(_randomIndexes[_idx]);
		if (_idx + 1 == this->mediaCount()) {
			_idx = 0;
		} else {
			_idx++;
		}
	} else {
		QMediaPlaylist::next();
	}
}

