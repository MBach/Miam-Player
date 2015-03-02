#ifndef GENERICDAO_H
#define GENERICDAO_H

#include <QObject>
#include "../miamcore_global.h"

/**
 * \brief		The GenericDAO class is a simple wrapper which contains basic informations about a file.
 * \author		Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY GenericDAO : public QObject
{
	Q_OBJECT
public:
	enum NodeType { Artist		= 0,
					Album		= 1,
					Track		= 2,
					Year		= 3,
					Undefined	= -1
				  };
private:
	QString _checksum, _host, _icon, _id, _title, _titleNormalized;

	GenericDAO *_parent;

	Q_ENUMS(NodeType)

	NodeType _type;

public:
	explicit GenericDAO(QObject *parentNode = 0, NodeType nt = Undefined);

	GenericDAO(const GenericDAO &remoteObject);

	GenericDAO& operator=(const GenericDAO& other);

	virtual ~GenericDAO();

	QString checksum() const;
	void setChecksum(const QString &checksum);

	QString host() const;
	void setHost(const QString &host);

	QString icon() const;
	void setIcon(const QString &icon);

	QString id() const;
	void setId(const QString &id);

	void setParentNode(GenericDAO *parentNode);
	GenericDAO* parentNode() const;

	QString title() const;
	void setTitle(const QString &title);

	QString titleNormalized() const;
	void setTitleNormalized(const QString &titleNormalized);

	NodeType type() const;
};

/** Register this class to convert in QVariant. */
Q_DECLARE_METATYPE(GenericDAO)

#endif // GENERICDAO_H
