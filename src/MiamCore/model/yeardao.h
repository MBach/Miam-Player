#ifndef YEARDAO_H
#define YEARDAO_H

#include "genericdao.h"

/**
 * \brief		The YearDAO class is a simple wrapper.
 * \author		Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY YearDAO : public GenericDAO
{
	Q_OBJECT
private:
	QString _year;

public:
	explicit YearDAO(QObject *parent = 0);

	YearDAO(const YearDAO &other);

	virtual ~YearDAO();

	QString year() const;
	void setYear(const QString &year);
};

/** Register this class to convert in QVariant. */
Q_DECLARE_METATYPE(YearDAO)

#endif // YEARDAO_H
