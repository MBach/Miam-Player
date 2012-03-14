#ifndef LIBRARYITEM_H
#define LIBRARYITEM_H

#include <QStandardItem>

#include "librarymodel.h"

class LibraryModel;

class LibraryItem : public QStandardItem
{
private:
	void setDisplayedName(const char *name, int size);

public:
	LibraryItem(const QString &text="");

	inline LibraryItem *child(int row, int column = 0) const { return (LibraryItem*) QStandardItem::child(row, column); }

	void setDisplayedName(const QString &name);
	void setFilePath(const QString &filePath);
	void setFilePath(int musicLocationIndex, const QString &fileName);
	void setMediaType(LibraryModel::MediaType mediaType);

	/** Should only be used for tracks. */
	void setRating(int rating);

	/** Should only be used for tracks. */
	void setTrackNumber(int trackNumber);

	/** Should only be used for albums or artists. */
	void setChildCount(int children);

	int mediaType() const;

	/** Reads data from the input stream and fills informations in this new node. */
	void read(QDataStream &in);

	/** Write data from this node to the output stream. */
	void write(QDataStream &out) const;

protected:
	inline void setData(const QVariant &value, int role) { QStandardItem::setData(value, role); }
};

#endif // LIBRARYITEM_H
