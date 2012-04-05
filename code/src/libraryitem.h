#ifndef LIBRARYITEM_H
#define LIBRARYITEM_H

#include <QStandardItem>

#include "librarymodel.h"

#include "libraryitemdelegate.h"

class LibraryModel;

/// Subclass in LibraryTrackItem, LibraryAlbumItem, LibraryArtist
/// and make this one virtual?
class LibraryItem : public QStandardItem
{
private:
	void setDisplayedName(const char *name, int size);

	LibraryItemDelegate *libraryItemDelegate;

	Q_ENUMS(CustomType)

public:
	LibraryItem(const QString &text="");

	~LibraryItem();

	enum CustomType { MEDIA_TYPE		= Qt::UserRole+2,
					  STAR_RATING		= Qt::UserRole+3,
					  CHILD_COUNT		= Qt::UserRole+4,
					  TRACK_NUMBER		= Qt::UserRole+5,
					  IDX_TO_ABS_PATH	= Qt::UserRole+6,
					  REL_PATH_TO_MEDIA	= Qt::UserRole+7
					};

	inline LibraryItem *child(int row, int column = 0) const { return (LibraryItem*) QStandardItem::child(row, column); }

	void setDisplayedName(const QString &name);
	void setFilePath(const QString &filePath);
	void setFilePath(int musicLocationIndex, const QString &fileName);
	void setMediaType(LibraryModel::MediaType mediaType);

	/** Should only be used for tracks. */
	void setRating(int rating);

	/** Should only be used for tracks. */
	void setTrackNumber(int trackNumber);
	inline int trackNumber() const { return data(TRACK_NUMBER).toInt(); }

	/** Should only be used for albums or artists. */
	void setChildCount(int children);

	int mediaType() const;

	/** Reads data from the input stream and fills informations in this new node. */
	void read(QDataStream &in);

	/** Write data from this node to the output stream. */
	void write(QDataStream &out) const;

	inline void setDelegate(LibraryItemDelegate *libraryItemDelegate) { this->libraryItemDelegate = libraryItemDelegate; }

	inline LibraryItemDelegate *itemDelegate() const { return this->libraryItemDelegate; }

protected:
	inline void setData(const QVariant &value, int role) { QStandardItem::setData(value, role); }
};

#endif // LIBRARYITEM_H
