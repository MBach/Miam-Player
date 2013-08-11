#ifndef PERSISTENTITEM_H
#define PERSISTENTITEM_H

#include "libraryitemtrack.h"

class PersistentItem : public LibraryItemTrack
{
private:
	void setPersistentItem(PersistentItem *persistentItem);
public:
	explicit PersistentItem() : LibraryItemTrack() {}

	explicit PersistentItem(const LibraryItemTrack *track);

	inline void setAlbum(const QString &album) { setData(album, ALBUM); }
	inline QString album() const { return data(ALBUM).toString(); }

	inline void setArtist(const QString &artist) { setData(artist, ARTIST); }
	inline QString artist() const { return data(ARTIST).toString(); }

	inline void setArtistAlbum(const QString &artistAlbum) { setData(artistAlbum, ARTISTALBUM); }
	inline QString artistAlbum() const { return data(ARTISTALBUM).toString(); }

	inline void setCoverFileName(const QString &cover) { setData(cover, COVER_FILENAME);}
	inline QString coverFileName() const { return data(COVER_FILENAME).toString(); }

	inline void setYear(int year) { setData(year, YEAR); }
	inline int year() const { return data(YEAR).toInt(); }

	/** Reads data from the input stream and fills informations in this new node. */
	void read(QDataStream &in);

	/** Write data from this node to the output stream. */
	void write(QDataStream &out) const;
};

#endif // PERSISTENTITEM_H
