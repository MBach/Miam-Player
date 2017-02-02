#ifndef COVERFETCHER_H
#define COVERFETCHER_H

#include <model/selectedtracksmodel.h>

#include "providers/coverartprovider.h"
#include "miamcoverfetcher_global.hpp"

/**
 * \brief       Fetch covers using MusicBrainz.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCOVERFETCHER_LIBRARY CoverFetcher : public QObject
{
	Q_OBJECT

private:
	QMap<QString, SelectedTracksModel*> _viewModels;

	QList<CoverArtProvider*> _providers;
	QNetworkAccessManager *_manager;

public:
	explicit CoverFetcher(QObject *parent = nullptr);

	virtual ~CoverFetcher();

	void fetch(SelectedTracksModel *selectedTracksModel);

private:
	/** When one is checking items in the list, providers are added or removed dynamically. */
	void manageProvider(bool enabled, QCheckBox *checkBox);
};

#endif // COVERFETCHER_H
