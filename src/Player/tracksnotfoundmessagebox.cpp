#include "tracksnotfoundmessagebox.h"

#include <QApplication>

TracksNotFoundMessageBox::TracksNotFoundMessageBox(QWidget *parent) :
	QMessageBox(parent)
{
	setIcon(QMessageBox::Warning);
	setWindowTitle(tr("Warning"));
}

void TracksNotFoundMessageBox::displayError(Context /*context*/, const QList<QMediaContent> &tracksNotFound)
{
	QString message;
	QString detailedMessage;
	if (tracksNotFound.size() == 1) {
		message = tr("The following track in your last playlists couldn't be found. It has probably moved to another location.");
	} else {
		message = tr("The following tracks in your last playlists couldn't be found. They have probably moved to another location.");
	}
	message.append("<ul>");
	for (int i=0; i < tracksNotFound.size(); i++) {
		QMediaContent trackNotFound = tracksNotFound.at(i);
		if (i < 10) {
			message.append("<li>" + trackNotFound.canonicalUrl().toLocalFile() + "</li>");
		} else {
			detailedMessage.append(trackNotFound.canonicalUrl().toLocalFile() + '\n');
		}
	}
	if (tracksNotFound.size() > 10) {
		message.append(tr("More tracks were not found."));
		this->setDetailedText(detailedMessage);
	}
	message.append("</ul>");
	this->setText(message);
	this->open();
}
