#ifndef TAGCONVERTER_H
#define TAGCONVERTER_H

#include <QDialog>

#include "ui_tagconverter.h"

/**
 * \brief		The TagConverter class displays a small popup to help one to extract Tag into files and vice-versa.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class TagConverter : public QDialog, public Ui::TagConverter
{
	Q_OBJECT

public:
	TagConverter(QWidget *parent = 0);
};

#endif // TAGCONVERTER_H
