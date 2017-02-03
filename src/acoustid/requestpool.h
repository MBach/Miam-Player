#ifndef REQUESTPOOL_H
#define REQUESTPOOL_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QUrlQuery>

#include "miamacoustid_global.hpp"
#include "mbrelease.h"

/**
 * \brief		The RequestPool class is used to limite rate to webservice.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMACOUSTID_LIBRARY RequestPool : public QObject
{
	Q_OBJECT
private:
	static int _maxRequestPerSecond;
	static int _nbRequestsToAcoustIdServer;

	QNetworkAccessManager _nam;
	QTimer *_timer;
	QMap<QString, QNetworkReply*> _map;

	/**
	 * \brief	The Quadruplet class is a nested class to manipulate queries easily.
	 */
	class Quadruplet
	{
	public:
		QString track;
		QNetworkRequest request;
		QUrlQuery urlQuery;
		int trackDuration;

		Quadruplet(const QString &tr, QNetworkRequest r, QUrlQuery u, int t) : track(tr), request(r), urlQuery(u), trackDuration(t) {}
	};

	QList<Quadruplet> _pool;

public:
	RequestPool(QObject *parent);

	void add(const QString &track, const QNetworkRequest &request, const QUrlQuery &urlQuery, int trackDuration);

private slots:
	void dispatchReply(QNetworkReply *reply);

signals:
	void releaseFound(const MusicBrainz::Release &);
	void tracksAnalyzed();
};

#endif // REQUESTPOOL_H
