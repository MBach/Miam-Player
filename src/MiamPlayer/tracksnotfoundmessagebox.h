#ifndef TRACKSNOTFOUNDMESSAGEBOX_H
#define TRACKSNOTFOUNDMESSAGEBOX_H

#include <QMediaContent>
#include <QMessageBox>

/**
 * \brief		The TracksNotFoundMessageBox class
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class TracksNotFoundMessageBox : public QMessageBox
{
	Q_OBJECT
public:
	enum Context { RESTORE_AT_STARTUP, ADDED_BY_ONE };

	TracksNotFoundMessageBox(QWidget *parent);

	void displayError(Context context, const QList<QMediaContent> &tracksNotFound);
};

#endif // TRACKSNOTFOUNDMESSAGEBOX_H
