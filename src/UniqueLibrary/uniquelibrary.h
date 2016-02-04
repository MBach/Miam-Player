#ifndef UNIQUELIBRARY_H
#define UNIQUELIBRARY_H

#include <QTranslator>

#include <abstractview.h>
#include "miamuniquelibrary_global.hpp"
#include "model/sqldatabase.h"

#include "ui_uniquelibrary.h"

/**
 * \brief		The UniqueLibrary class
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMUNIQUELIBRARY_LIBRARY UniqueLibrary : public AbstractView, public Ui::uniqueLibrary
{
	Q_OBJECT
private:
	MediaPlayer *_mediaPlayer;
	QStandardItem *_currentTrack;
	UniqueLibraryFilterProxyModel *_proxy;

	QTranslator translator;

	QModelIndexList _randomHistoryList;

public:
	explicit UniqueLibrary(MediaPlayer *mediaPlayer, QWidget *parent = nullptr);

	virtual ~UniqueLibrary();

	virtual bool hasTracksToDisplay() const override;

	virtual bool viewProperty(SettingsPrivate::ViewProperty vp) const override;

protected:
	virtual void changeEvent(QEvent *event) override;

	virtual void closeEvent(QCloseEvent *event) override;

public slots:
	virtual void setMusicSearchEngine(MusicSearchEngine *musicSearchEngine) override;

	virtual void setViewProperty(SettingsPrivate::ViewProperty vp, QVariant value) override;

	virtual void volumeSliderDecrease() override;

	virtual void volumeSliderIncrease() override;

private slots:
	bool playSingleTrack(const QModelIndex &index);

	void skipBackward();

	void skipForward();

	void toggleShuffle();
};

#endif // UNIQUELIBRARY_H
