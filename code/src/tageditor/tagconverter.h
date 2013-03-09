#ifndef TAGCONVERTER_H
#define TAGCONVERTER_H

#include <QDialog>

#include "ui_tagconverter.h"

class TagConverter : public QDialog, public Ui::TagConverter
{
	Q_OBJECT

public:
	TagConverter(QWidget *parent = 0);
};

#endif // TAGCONVERTER_H
