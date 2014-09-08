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
	QPushButton *_convertButton;
	TagEditorTableWidget *_tagEditor;

public:
	explicit TagConverter(QPushButton *convertButton, TagEditorTableWidget *parent);

	void setVisible(bool b);

protected:
	virtual bool eventFilter(QObject *, QEvent *event);

private:
	QString autoGuessPatternFromFile() const;

	QString generatePattern(TagLineEdit *lineEdit) const;

private slots:
	void applyPatternToColumns();

	void applyPatternToFilenames();
};

#endif // TAGCONVERTER_H
