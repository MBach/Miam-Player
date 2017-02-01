#ifndef MBRELEASE_H
#define MBRELEASE_H

#include <QMap>
#include <QObject>

namespace MusicBrainz {

/**
 * \brief The Artist class
 */
class Artist : public QObject
{
	Q_OBJECT
public:
	QString id;
	QString name;

	/** Default constructor. */
	Artist(QObject *parent = nullptr) : QObject(parent) {}

	/** Default copy constructor. */
	Artist(const Artist &other);

	/** Default destructor. */
	virtual ~Artist() {}

	Artist& operator=(const Artist& other);
};

inline bool operator==(Artist const &a1, Artist const &a2)
{
	return a1.id.compare(a2.id) == 0;
}


/**
 * \brief The Track class
 */
class Track : public QObject
{
	Q_OBJECT
public:
	QString id;
	int position;
	int number;
	int length;
	QString title;
	Artist *artist;

	/** Default constructor. */
	Track(QObject *parent = nullptr) : QObject(parent), artist(new Artist(this)) {}

	/** Default copy constructor. */
	Track(const Track &other);

	/** Default destructor. */
	virtual ~Track() {}

	Track& operator=(const Track& other);
};

inline bool operator==(Track const &d1, Track const &d2)
{
	return d1.id.compare(d2.id) == 0;
}


/**
 * \brief The Release class
 */
class Release : public QObject
{
	Q_OBJECT
public:
	QString id;
	QString releaseGroupId;
	int trackCount;
	QString title;
	QString country;
	int year;
	QString format;
	int disc;
	QMap<QString, Track> tracks;
	Artist artist;

	/** Default constructor. */
	Release(QObject *parent = nullptr) : QObject(parent) {}

	/** Default destructor. */
	virtual ~Release() {}

	/** Default copy constructor. */
	Release(const Release &mbRelease);

	Release& operator=(const Release& other);

	Track track(const QString &filename) const;
};

inline bool operator==(Release const &d1, Release const &d2)
{
	return d1.id.compare(d2.id) == 0;
}

}

/** Register this class to convert in QVariant. */
Q_DECLARE_METATYPE(MusicBrainz::Track)
Q_DECLARE_METATYPE(MusicBrainz::Artist)
Q_DECLARE_METATYPE(MusicBrainz::Release)

#endif // MBRELEASE_H
