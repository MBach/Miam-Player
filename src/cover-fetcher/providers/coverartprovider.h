#ifndef COVERARTPROVIDER_H
#define COVERARTPROVIDER_H

#include <QNetworkReply>
#include <QObject>
#include <QUrl>

#include "miamcoverfetcher_global.hpp"

/**
 * \brief		The CoverArtProvider class is an abstract class for registering various webservices.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCOVERFETCHER_LIBRARY CoverArtProvider : public QObject
{
	Q_OBJECT
private:
	Q_ENUMS(Fetch_Operations)
	Q_ENUMS(ProviderType)

protected:
	QNetworkAccessManager *_manager;

public:
	enum Fetch_Operations : int
	{
		FO_GetReleases		= 0,
		FO_DownloadCover	= 1,
		FO_Search			= 2
	};

	enum ProviderType : int
	{
		PT_MusicBrainz	= 0,
		PT_Amazon		= 1,
		PT_Discogs		= 2,
		PT_LastFM		= 3
	};

	explicit CoverArtProvider(QNetworkAccessManager *manager) : QObject(manager), _manager(manager) {}

	virtual QUrl query(const QString &artist, const QString &album) = 0;

	virtual QUrl album(const QString &) = 0;

	virtual ProviderType type() = 0;

public slots:
	virtual void dispatchReply(QNetworkReply *reply) = 0;

signals:
	void aboutToCreateCover(const QString &album, const QByteArray &coverByteArray);
};

#endif // COVERARTPROVIDER_H
