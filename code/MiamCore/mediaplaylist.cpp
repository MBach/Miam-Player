#include "mediaplaylist.h"

#include <algorithm>
#include <ctime>
#include <random>

#include <QtDebug>

MediaPlaylist::MediaPlaylist(QObject *parent)
	: QMediaPlaylist(parent), _idx(0)
{
	connect(this, &QMediaPlaylist::playbackModeChanged, this, [=](PlaybackMode mode) {
		if (!isEmpty()) {
			if (mode == Random) {
				this->createRandom();
			} else {
				this->resetRandom();
			}
		}
	});
}

void MediaPlaylist::shuffle(int idx)
{
	this->resetRandom();
	this->createRandom();
	if (idx == -1) {
		return;
	}
	for (uint i = 0; i < _randomIndexes.size(); i++) {
		if (idx == _randomIndexes[i]) {
			_idx = i;
			break;
		}
	}
	this->setCurrentIndex(idx);
}

void MediaPlaylist::skipBackward()
{
	if (playbackMode() == Random) {
		_idx--;
		if (_idx < 0) {
			_idx = this->mediaCount() - 1;
		}

		this->setCurrentIndex(_randomIndexes[_idx]);
	} else {
		this->previous();
	}
}

void MediaPlaylist::skipForward()
{
	if (playbackMode() == Random) {
		if (_idx + 1 == this->mediaCount()) {
			_idx = 0;
		} else {
			_idx++;
		}
		this->setCurrentIndex(_randomIndexes[_idx]);
	} else {
		this->next();
	}
}

void MediaPlaylist::createRandom()
{
	for (int i = 0; i < mediaCount(); i++) {
		_randomIndexes.push_back(i);
	}

	std::srand(std::time(nullptr));
	std::random_shuffle(_randomIndexes.begin(), _randomIndexes.end());
	// Pick first item, start playback with:
	_idx = _randomIndexes[0];
}

void MediaPlaylist::resetRandom()
{
	_randomIndexes.clear();
	_idx = 0;
}
