#ifndef TRACKSNOTFOUNDMESSAGEBOX_H
#define TRACKSNOTFOUNDMESSAGEBOX_H

#include <QMessageBox>

class TracksNotFoundMessageBox : public QMessageBox
{
	Q_OBJECT
public:
	enum Context { RESTORE_AT_STARTUP, ADDED_BY_ONE };

	TracksNotFoundMessageBox(QWidget *parent);

	void displayError(Context context, const QStringList &tracksNotFound);
};

#endif // TRACKSNOTFOUNDMESSAGEBOX_H
