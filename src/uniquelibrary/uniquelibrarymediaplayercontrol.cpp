#include "uniquelibrarymediaplayercontrol.h"

#include <settingsprivate.h>
#include "uniquelibrary.h"

UniqueLibraryMediaPlayerControl::UniqueLibraryMediaPlayerControl(MediaPlayer *mediaPlayer, QWidget *parent)
	: AbstractMediaPlayerControl(mediaPlayer, parent)
	, _uniqueLibrary(nullptr)
{

}

bool UniqueLibraryMediaPlayerControl::isInShuffleState() const
{
	return SettingsPrivate::instance()->value("uniqueLibraryIsInShuffleState").toBool();
}

void UniqueLibraryMediaPlayerControl::skipBackward()
{
	if (!_uniqueLibrary->currentTrack()) {
		return;
	}
	mediaPlayer()->blockSignals(true);

	if (_uniqueLibrary->playbackModeButton->isChecked()) {
		if (_uniqueLibrary->randomHistoryList()->isEmpty()) {
			return;
		} else {
			_uniqueLibrary->playSingleTrack(_uniqueLibrary->randomHistoryList()->takeLast());
		}
	} else {
		QModelIndex current = _uniqueLibrary->proxy()->mapFromSource(_uniqueLibrary->uniqueTable->model()->index(_uniqueLibrary->currentTrack()->row(), 1));
		int row = current.row();
		while (row >= 0) {
			QModelIndex previous = current.sibling(row - 1, 1);
			if (_uniqueLibrary->playSingleTrack(previous)) {
				_uniqueLibrary->uniqueTable->scrollTo(previous);
				break;
			} else {
				row--;
			}
		}
	}
	mediaPlayer()->blockSignals(false);
}

void UniqueLibraryMediaPlayerControl::skipForward()
{
	mediaPlayer()->blockSignals(true);

	if (_uniqueLibrary->currentTrack()) {
		_uniqueLibrary->currentTrack()->setData(false, Miam::DF_Highlighted);

		// Append to random history the track the player is playing
		if (_uniqueLibrary->playbackModeButton->isChecked()) {
			_uniqueLibrary->randomHistoryList()->append(_uniqueLibrary->proxy()->mapFromSource(_uniqueLibrary->currentTrack()->index()));
		}
	}

	if (_uniqueLibrary->playbackModeButton->isChecked()) {
		int rows = _uniqueLibrary->uniqueTable->model()->rowCount();
		if (rows > 0) {
			int r = rand() % rows;
			QModelIndex idx = _uniqueLibrary->uniqueTable->model()->index(r, 1);
			while (_uniqueLibrary->uniqueTable->model()->itemFromIndex(idx)->type() != Miam::IT_Track) {
				idx = _uniqueLibrary->uniqueTable->model()->index(rand() % rows, 1);
			}
			QModelIndex next = _uniqueLibrary->proxy()->mapFromSource(idx);
			_uniqueLibrary->playSingleTrack(next);
		}
	} else {
		QModelIndex current;
		if (_uniqueLibrary->currentTrack()) {
			current = _uniqueLibrary->proxy()->mapFromSource(_uniqueLibrary->uniqueTable->model()->index(_uniqueLibrary->currentTrack()->row(), 1));
		} else {
			current = _uniqueLibrary->proxy()->index(0, 1);
		}
		int row = current.row();
		while (row < _uniqueLibrary->uniqueTable->model()->rowCount()) {
			QModelIndex next = current.sibling(row + 1, 1);
			if (_uniqueLibrary->playSingleTrack(next)) {
				break;
			} else {
				row++;
			}
		}
	}
	mediaPlayer()->blockSignals(false);
}

void UniqueLibraryMediaPlayerControl::stop()
{
	if (_uniqueLibrary->currentTrack()) {
		_uniqueLibrary->currentTrack()->setData(false, Miam::DF_Highlighted);
	}
	mediaPlayer()->stop();
}

void UniqueLibraryMediaPlayerControl::togglePlayback()
{
	if (_uniqueLibrary->currentTrack() && mediaPlayer()->state() == QMediaPlayer::StoppedState) {
		_uniqueLibrary->playSingleTrack(_uniqueLibrary->proxy()->mapFromSource(_uniqueLibrary->currentTrack()->index()));
	} else {
		mediaPlayer()->togglePlayback();
	}
}

void UniqueLibraryMediaPlayerControl::toggleShuffle(bool checked)
{
	_uniqueLibrary->playbackModeButton->setChecked(checked);
	SettingsPrivate::instance()->setValue("uniqueLibraryIsInShuffleState", checked);
	if (!checked) {
		_uniqueLibrary->randomHistoryList()->clear();
	}
}
