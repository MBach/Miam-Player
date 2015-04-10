#ifndef LIBRARYORDERDIALOG_H
#define LIBRARYORDERDIALOG_H

#include <QDialog>
#include "ui_libraryorderdialog.h"

/**
 * \brief		The LibraryOrderDialog class displays a small popup which allows one to change the hierarchy
 * \details     Four modes are built-in: Artist \ Album \ Tracks ; Artist - Album \ Tracks ; Album \ Tracks ; Year \ Artist - Album \ Tracks.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class LibraryOrderDialog : public QDialog, public Ui::LibraryOrderDialog
{
	Q_OBJECT
public:
	explicit LibraryOrderDialog(QWidget *parent = 0);

	QString headerValue() const;

	void setVisible(bool b);
};

#endif // LIBRARYORDERDIALOG_H
