#ifndef LIBRARYITEM_H
#define LIBRARYITEM_H

#include <QStandardItem>

class LibraryItem : public QStandardItem
{
private:
	Q_ENUMS(CustomType)
	Q_ENUMS(MediaType)

public:
	/// Constructor with a title and a filetype.
	explicit LibraryItem(const QString &text);

	explicit LibraryItem() : QStandardItem() {}

	enum MediaType { Artist		= QStandardItem::UserType + 1,
					 Album		= QStandardItem::UserType + 2,
					 Track		= QStandardItem::UserType + 3,
					 Letter		= QStandardItem::UserType + 4
				   };

	  /// XXX: refactor (remove...) this one because of AbsFilePath?
	enum { SUFFIX = Qt::UserRole + 8 };

	enum { NormalizedString = Qt::UserRole + 10 };

	inline LibraryItem *child(int row, int column = 0) const { return (LibraryItem*) QStandardItem::child(row, column); }

	QString normalizedString() const { return data(NormalizedString).toString(); }

protected:
	QVariant data(int role) const { return QStandardItem::data(role); }
};

#endif // LIBRARYITEM_H
