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

	explicit LibraryItem();

	enum MediaType { Artist		= QStandardItem::UserType + 1,
					 Album		= QStandardItem::UserType + 2,
					 Track		= QStandardItem::UserType + 3,
					 Letter		= QStandardItem::UserType + 4
				   };

	enum CustomType { CHILD_COUNT		= Qt::UserRole + 4,
					  TRACK_NUMBER		= Qt::UserRole + 5,
					  //IDX_TO_ABS_PATH	= Qt::UserRole + 6,
					  //REL_PATH_TO_MEDIA	= Qt::UserRole + 7,
					  /// XXX: refactor (remove...) this one
					  SUFFIX			= Qt::UserRole + 8,
					  FILEPATH			= Qt::UserRole + 9
					};

	enum { NormalizedString = Qt::UserRole + 10 };

	inline LibraryItem *child(int row, int column = 0) const { return (LibraryItem*) QStandardItem::child(row, column); }

	virtual int childCount() const { return data(LibraryItem::CHILD_COUNT).toInt(); }

	QString normalizedString() const { return data(NormalizedString).toString(); }

protected:
	void setDisplayedName(const QString &name);

	QVariant data(int role) const { return QStandardItem::data(role); }
};

#endif // LIBRARYITEM_H
