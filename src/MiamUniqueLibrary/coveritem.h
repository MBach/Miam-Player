#ifndef COVERITEM_H
#define COVERITEM_H

#include "miamuniquelibrary_global.hpp"
#include <QStandardItem>

/**
 * \brief		The CoverItem class holds the cover path to display an image into the view.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
*/
class MIAMUNIQUELIBRARY_LIBRARY CoverItem : public QStandardItem
{
public:
	CoverItem(const QString &coverPath);

	virtual int type() const override;
};

#endif // COVERITEM_H
