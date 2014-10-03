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
private:
	QString _checksum, _host, _id;

public:
	explicit GenericDAO(QObject *parent = 0);

	GenericDAO(const GenericDAO &remoteObject);

	virtual ~GenericDAO();

	QString checksum() const;
	void setChecksum(const QString &checksum);

	QString host() const;
	void setHost(const QString &host);

	QString id() const;
	void setId(const QString &id);
};

#endif // GENERICDAO_H
