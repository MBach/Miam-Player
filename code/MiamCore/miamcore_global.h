#ifndef MIAMCORE_GLOBAL_H
#define MIAMCORE_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef MIAM_PLUGIN
# define MIAMCORE_LIBRARY Q_DECL_EXPORT
#else
# define MIAMCORE_LIBRARY Q_DECL_IMPORT
#endif

#include <QMetaType>
#include <QObject>

class MIAMCORE_LIBRARY Miam : public QObject
{
	Q_OBJECT
private:
	Q_ENUMS(ItemType)
	Q_ENUMS(DataField)

public:
	enum ItemType : int
	{
		IT_Artist		= QMetaType::User + 1,
		IT_Album		= QMetaType::User + 2,
		IT_ArtistAlbum		= QMetaType::User + 3,
		IT_Disc			= QMetaType::User + 4,
		IT_Separator		= QMetaType::User + 5,
		IT_Track		= QMetaType::User + 6,
		IT_Year			= QMetaType::User + 7,
		IT_Playlist		= QMetaType::User + 8,
		IT_UnknownType		= QMetaType::User + 9
	};

	// User defined data types (item->setData(QVariant, Field);)
	enum DataField : int
	{
		DF_URI			= Qt::UserRole + 1,
		DF_CoverPath		= Qt::UserRole + 2,
		DF_TrackNumber		= Qt::UserRole + 3,
		DF_DiscNumber		= Qt::UserRole + 4,
		DF_NormalizedString	= Qt::UserRole + 5,
		DF_Year			= Qt::UserRole + 6,
		DF_Highlighted		= Qt::UserRole + 7,
		DF_IsRemote		= Qt::UserRole + 8,
		DF_IconPath		= Qt::UserRole + 9,
		DF_Rating		= Qt::UserRole + 10,
		DF_CustomDisplayText	= Qt::UserRole + 11
	};
};

#endif // MIAMCORE_GLOBAL_H
