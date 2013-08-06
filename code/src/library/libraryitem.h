#ifndef LIBRARYITEM_H
#define LIBRARYITEM_H

#include <QStandardItem>

class LibraryItem : public QStandardItem
{
private:
	Q_ENUMS(CustomType)
	Q_ENUMS(MediaType)

public:
	enum CustomType { TITLE				= Qt::DisplayRole,
					  ABSOLUTE_PATH		= Qt::UserRole + 1,
					  ALBUM				= Qt::UserRole + 2,
					  ARTIST			= Qt::UserRole + 3,
					  ARTISTALBUM		= Qt::UserRole + 4,
					  COVER_FILENAME	= Qt::UserRole + 5,
					  FILENAME			= Qt::UserRole + 6,
					  TRACK_NUMBER		= Qt::UserRole + 7,
					  YEAR				= Qt::UserRole + 8
					};

	explicit LibraryItem() : QStandardItem() {}

	/// Constructor with a title and a filetype.
	explicit LibraryItem(const QString &text);

	enum MediaType { Artist		= QStandardItem::UserType + 1,
					 Album		= QStandardItem::UserType + 2,
					 Track		= QStandardItem::UserType + 3,
					 Letter		= QStandardItem::UserType + 4
				   };

	enum { NormalizedString = Qt::UserRole + 9 };

	inline LibraryItem *child(int row, int column = 0) const { return (LibraryItem*) QStandardItem::child(row, column); }

	QString normalizedString() const { return data(NormalizedString).toString(); }

protected:
	QVariant data(int role) const { return QStandardItem::data(role); }
};

#endif // LIBRARYITEM_H
