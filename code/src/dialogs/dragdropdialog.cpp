#include "dragdropdialog.h"

#include <QFileInfo>
#include <QRadioButton>

#include "settings.h"

#include <QtDebug>

DragDropDialog::DragDropDialog(QWidget *parent) :
	QDialog(parent)
{
	setupUi(this);

	connect(toolButtonLibrary, SIGNAL(clicked()), this, SLOT(addExternalFoldersToLibrary()));
	connect(toolButtonPlaylist, SIGNAL(clicked()), this, SLOT(addExternalFoldersToPlaylist()));

	originalLabel = labelHowToProceed->text();
}

void DragDropDialog::setMimeData(const QMimeData *mimeData)
{
	if (mimeData->hasUrls()) {
		QList<QUrl> urlList = mimeData->urls();
		QString newLabel;
		int folders = 0;
		int maxDisplayedInLabel = 3;

		_externalLocations.clear();
		labelHowToProceed->setText(originalLabel);

		for (int i = 0; i < urlList.size(); i++) {
			QFileInfo fileInfo = urlList.at(i).toLocalFile();
			if (fileInfo.isDir()) {
				// Builds the label as a concatenation of folders' name
				if (folders < maxDisplayedInLabel) {
					newLabel.append(fileInfo.fileName()).append(", ");
					folders++;
				}
				_externalLocations.append(fileInfo.absoluteFilePath());
			}
		}
		if (newLabel.length() > 2) {
			newLabel = newLabel.left(newLabel.length() - 2);
			if (folders >= maxDisplayedInLabel) {
				newLabel.append(", ... ");
			}
			labelHowToProceed->setText(labelHowToProceed->text().arg(newLabel));
		}
	}
}

void DragDropDialog::addExternalFoldersToLibrary()
{
	if (checkBoxRememberChoice->isChecked()) {
		emit rememberDragDrop(toolButtonLibrary);
	}
	emit aboutToAddExtFoldersToLibrary(_externalLocations);
	this->accept();
}

void DragDropDialog::addExternalFoldersToPlaylist()
{
	if (checkBoxRememberChoice->isChecked()) {
		emit rememberDragDrop(toolButtonPlaylist);
	}
	qDebug() << _externalLocations.size();
	emit aboutToAddExtFoldersToPlaylist(_externalLocations);
	this->accept();
}
