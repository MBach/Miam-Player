#ifndef TAGCONVERTER_H
#define TAGCONVERTER_H

#include <QDialog>

#include "ui_tagconverter.h"

class TagEditorTableWidget;

/**
 * \brief		The TagConverter class displays a small popup to help one to extract Tag into files and vice-versa.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class TagConverter : public QDialog, public Ui::TagConverter
{
	Q_OBJECT

private:
	TagEditorTableWidget *_tagEditor;

public:
	explicit TagConverter(TagEditorTableWidget *parent);

private:
	QString generatePattern(TagLineEdit *lineEdit) const;

signals:
	void aboutToApplyPattern(int column, const QString &data);
};

#endif // TAGCONVERTER_H
