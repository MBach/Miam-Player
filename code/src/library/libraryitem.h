#ifndef LIBRARYITEM_H
#define LIBRARYITEM_H

#include <QStandardItem>

#include "librarymodel.h"

class LibraryModel;

/// Subclass in LibraryTrackItem, LibraryAlbumItem, LibraryArtist
/// and make this one virtual?
class LibraryItem : public QStandardItem
{
private:
	void setDisplayedName(const char *name, int size);

	Q_ENUMS(CustomType)

public:
	/// Constructor with a title and a filetype.
	LibraryItem(const QString &text, LibraryModel::MediaType mediaType, int type=-1);

	LibraryItem();

	enum CustomType { INTERNAL_NAME		= Qt::UserRole+1,
					  MEDIA_TYPE		= Qt::UserRole+2,
					  STAR_RATING		= Qt::UserRole+3,
					  CHILD_COUNT		= Qt::UserRole+4,
					  TRACK_NUMBER		= Qt::UserRole+5,
					  IDX_TO_ABS_PATH	= Qt::UserRole+6,
					  REL_PATH_TO_MEDIA	= Qt::UserRole+7,
					  SUFFIX			= Qt::UserRole+8
					};

	inline LibraryItem *child(int row, int column = 0) const { return (LibraryItem*) QStandardItem::child(row, column); }

	void setFilePath(const QString &filePath);
	void setFilePath(int musicLocationIndex, const QString &fileName);

	/** Should only be used for tracks. */
	void setRating(int rating);

	/** Should only be used for tracks. */
	void setTrackNumber(int trackNumber);
	inline int trackNumber() const { return data(TRACK_NUMBER).toInt(); }

	/** Redefined for custom types (greater than Qt::UserRole). */
	int type() const;

	/** Reads data from the input stream and fills informations in this new node. */
	void read(QDataStream &in);

	/** Write data from this node to the output stream. */
	void write(QDataStream &out) const;

private:
	void setMediaType(LibraryModel::MediaType mediaType);

//protected:
	//inline void setData(const QVariant &value, int role) { QStandardItem::setData(value, role); }
};

#endif // LIBRARYITEM_H
