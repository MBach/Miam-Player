#include "tabplaylist.h"
#include "settings.h"
#include "libraryitem.h"

#include <QApplication>
#include <QMessageBox>

/** Default constructor. */
TabPlaylist::TabPlaylist(QWidget *parent) :
    QTabWidget(parent)
{
	this->setTabsClosable(true);
	this->tabBar()->addTab(QIcon(":/icons/plusIcon"), QString());
	this->tabBar()->setTabButton(count()-1, QTabBar::RightSide, 0);

	// Init Phonon Module
	mediaObject = new MediaObject(this);
	metaInformationResolver = new MediaObject(this);

	// Link core mp3 actions
	connect(mediaObject, SIGNAL(stateChanged(Phonon::State, Phonon::State)), this, SLOT(stateChanged(Phonon::State, Phonon::State)));
	connect(metaInformationResolver, SIGNAL(stateChanged(Phonon::State, Phonon::State)), this, SLOT(metaStateChanged(Phonon::State, Phonon::State)));
	connect(mediaObject, SIGNAL(finished()), this, SLOT(skipForward()));

	// Other actions
	connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(removeTabFromCloseButton(int)));
}

/** Action sent from the menu. */
void TabPlaylist::removeCurrentPlaylist()
{
	removeTabFromCloseButton(this->tabBar()->currentIndex());
}

/** Remove a playlist when clicking on a close button in the corner. */
void TabPlaylist::removeTabFromCloseButton(int index)
{
	// Don't delete the first tab, if it's the last one remaining
	if (index > 0 || (index == 0 && count() > 2)) {
		// If the playlist we want to delete has no more right tab, then pick the left tab
		if (index+1 > count()-2) {
			setCurrentIndex(index-1);
		}
		this->removeTab(index);
		delete this->widget(index);
	} else {
		// Clear the content of last tab
		currentPlayList()->clear();
	}
}

/** Add a track from the filesystem or the library to the current playlist. */
QTableWidgetItem * TabPlaylist::addItemToCurrentPlaylist(const QPersistentModelIndex &itemFromLibrary)
{
	QTableWidgetItem *index = NULL;
	if (itemFromLibrary.isValid()) {
		QString filePath = Settings::getInstance()->musicLocations().at(itemFromLibrary.data(LibraryItem::IDX_TO_ABS_PATH).toInt()).toString();
		QString fileName = itemFromLibrary.data(LibraryItem::REL_PATH_TO_MEDIA).toString();
		MediaSource source(filePath + fileName);
		if (source.type() != MediaSource::Invalid) {
			index = currentPlayList()->append(source);
			if (currentPlayList()->tracks()->size() == 1) {
				metaInformationResolver->setCurrentSource(currentPlayList()->tracks()->at(0));
			}
		}
	}
	return index;
}

/** Convenient getter with cast. */
Playlist * TabPlaylist::currentPlayList() const
{
	Playlist *p = qobject_cast<Playlist *>(this->currentWidget());
	return p;
}

void TabPlaylist::stateChanged(State newState, State oldState)
{
	switch (newState) {
	case ErrorState:
		if (mediaObject->errorType() == FatalError) {
			QMessageBox::warning(this, tr("Fatal Error"),
			mediaObject->errorString());
		} else {
			QMessageBox::warning(this, tr("Error"),
			mediaObject->errorString());
		}
		break;

	case BufferingState:
		break;

	case StoppedState:
		if (oldState == LoadingState || oldState == PausedState) {
			mediaObject->play();
		}
		break;
	default:
		;
	}
}

/** When the user is double clicking on a track in a playlist. */
void TabPlaylist::changeTrack(QTableWidgetItem *item)
{
	MediaSource media = currentPlayList()->tracks()->at(item->row());
	currentPlayList()->setActiveTrack(item->row());
	mediaObject->setCurrentSource(media);
}

/** Change the current track to the previous one. */
void TabPlaylist::skipBackward()
{
	int activeTrack = currentPlayList()->activeTrack();
	if (activeTrack-- > 0) {
		QTableWidgetItem *item = currentPlayList()->table()->item(activeTrack, 1);
		this->changeTrack(item);
	}
}

/** Change the current track to the next one. */
void TabPlaylist::skipForward()
{
	int activeTrack = currentPlayList()->activeTrack();
	if (++activeTrack < currentPlayList()->tracks()->size()) {
		QTableWidgetItem *item = currentPlayList()->table()->item(activeTrack, 1);
		this->changeTrack(item);
	}
}

void TabPlaylist::addItemFromLibraryToPlaylist(const QPersistentModelIndex &item)
{
	bool isEmpty = currentPlayList()->tracks()->isEmpty();
	QTableWidgetItem *indexInPlaylist = this->addItemToCurrentPlaylist(item);
	if (isEmpty) {
		this->changeTrack(indexInPlaylist);
	}
}

void TabPlaylist::metaStateChanged(State newState, State /* oldState */)
{
	if (newState == ErrorState) {
		QMessageBox::warning(this, tr("Error opening files"), metaInformationResolver->errorString());
		//while (!sources.isEmpty() && !(sources.takeLast() == metaInformationResolver->currentSource())) {
		// TODO
		//}  /* loop */;
		return;
	}

	if (newState != StoppedState && newState != PausedState) {
		return;
	}

	if (metaInformationResolver->currentSource().type() == MediaSource::Invalid) {
		return;
	}

	if (currentPlayList()->table()->selectedItems().isEmpty()) {
		currentPlayList()->table()->selectRow(0);
		mediaObject->setCurrentSource(metaInformationResolver->currentSource());
	}

	/// TODO: code review
	int index = currentPlayList()->activeTrack();
	if (currentPlayList()->tracks()->size() > index) {
		//metaInformationResolver->setCurrentSource(currentPlayList()->tracks()->at(index));
	} else {
		currentPlayList()->table()->resizeColumnsToContents();
		if (currentPlayList()->table()->columnWidth(0) > 300) {
			currentPlayList()->table()->setColumnWidth(0, 300);
		}
	}
}

void TabPlaylist::tick(qint64 time)
{
	//QTime displayTime(0, (time / 60000) % 60, (time / 1000) % 60);
	// TODO
	//timeLcd->display(displayTime.toString("mm:ss"));
}

/** Retranslate tabs' name and all playlists in this widget. */
void TabPlaylist::retranslateUi()
{
	// No translation for the (+) tab button
	for (int i=0; i < count()-1; i++) {
		Playlist *p = qobject_cast<Playlist *>(widget(i));
		QString playlist = QApplication::translate("MainWindow", "Playlist ", 0, QApplication::UnicodeUTF8);
		playlist.append(QString::number(i+1));
		setTabText(i, playlist);
		p->retranslateUi();
	}
}
