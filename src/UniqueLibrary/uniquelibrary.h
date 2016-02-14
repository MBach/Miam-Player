#ifndef UNIQUELIBRARY_H
#define UNIQUELIBRARY_H

#include <QTranslator>

#include <model/sqldatabase.h>
#include <abstractview.h>
#include "uniquelibrarymediaplayercontrol.h"

#include "miamuniquelibrary_global.hpp"

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
	QStandardItem *_currentTrack;

	UniqueLibraryFilterProxyModel *_proxy;

	QTranslator translator;

	QModelIndexList *_randomHistoryList;

public:
	explicit UniqueLibrary(MediaPlayer *mediaPlayer, QWidget *parent = nullptr);

	virtual ~UniqueLibrary();

	inline QStandardItem* currentTrack() const { return _currentTrack; }

	inline UniqueLibraryFilterProxyModel* proxy() const { return _proxy; }

	inline QModelIndexList* randomHistoryList() const { return _randomHistoryList; }

	inline virtual QSize sizeHint() const override { return QSize(420, 850); }

	virtual bool viewProperty(Settings::ViewProperty vp) const override;

protected:
	virtual void changeEvent(QEvent *event) override;

	virtual void closeEvent(QCloseEvent *event) override;

public slots:
	bool playSingleTrack(const QModelIndex &index);

	virtual void setMusicSearchEngine(MusicSearchEngine *musicSearchEngine) override;

	virtual void setViewProperty(Settings::ViewProperty vp, QVariant value) override;

	virtual void volumeSliderDecrease() override;

	virtual void volumeSliderIncrease() override;
};

#endif // UNIQUELIBRARY_H
