#ifndef LIBRARYITEMTRACK_H
#define LIBRARYITEMTRACK_H

#include "libraryitem.h"

class LibraryItemTrack : public LibraryItem
{
private:
	Q_ENUMS(CustomType)

public:
	enum CustomType { TITLE				= Qt::DisplayRole,
					  ALBUM				= Qt::UserRole + 2,
					  ARTIST			= Qt::UserRole + 3,
					  ARTISTALBUM		= Qt::UserRole + 4,
					  FILEPATH			= Qt::UserRole + 5,
					  TRACK_NUMBER		= Qt::UserRole + 6,
					  YEAR				= Qt::UserRole + 7
					};

	explicit LibraryItemTrack() : LibraryItem() {}

	explicit LibraryItemTrack(const QString &track, int suffix);

	void setTrackNumber(int track) { setData(track, TRACK_NUMBER); }
	inline int trackNumber() const { return data(TRACK_NUMBER).toInt(); }

	/** Redefined for delegates (painting, etc). */
	inline int type() const { return LibraryItem::Track; }

	inline QString filePath() const { return data(FILEPATH).toString(); }
	inline void setFilePath(const QString &absFilePath) { setData(absFilePath, FILEPATH); }

	inline void setAlbum(const QString &album) { setData(album, ALBUM); }
	inline QString album() const { return data(ALBUM).toString(); }

	void setArtist(const QString &artist) { setData(artist, ARTIST); }
	inline QString artist() const { return data(ARTIST).toString(); }

	inline void setArtistAlbum(const QString &artistAlbum) { setData(artistAlbum, ARTISTALBUM); }
	inline QString artistAlbum() const { return data(ARTISTALBUM).toString(); }

	inline void setYear(int year) { setData(year, YEAR); }
	inline int year() const { return data(YEAR).toInt(); }

	/** Reads data from the input stream and fills informations in this new node. */
	void read(QDataStream &in);

	/** Write data from this node to the output stream. */
	void write(QDataStream &out) const;
};

#endif // LIBRARYITEMTRACK_H
