#include "dragdropdialog.h"

#include <QFileInfo>
#include <QRadioButton>

#include "settingsprivate.h"

DragDropDialog::DragDropDialog(QWidget *parent) :
	QDialog(parent)
{
	setupUi(this);

	connect(toolButtonLibrary, &QToolButton::clicked, this, &DragDropDialog::addExternalFoldersToLibrary);
	connect(toolButtonPlaylist, &QToolButton::clicked, this, &DragDropDialog::addExternalFoldersToPlaylist);

	originalLabel = labelHowToProceed->text();
}

bool DragDropDialog::setMimeData(const QMimeData *mimeData)
{
	externalLocations.clear();
	if (!mimeData->hasUrls()) {
		return false;
	}
	bool onlyFiles = true;
	QList<QUrl> urlList = mimeData->urls();
	QString newLabel;
	int folders = 0;
	int maxDisplayedInLabel = 3;

	labelHowToProceed->setText(originalLabel);

	for (int i = 0; i < urlList.size(); i++) {
		QFileInfo fileInfo = urlList.at(i).toLocalFile();
		if (fileInfo.isDir()) {
			// Builds the label as a concatenation of folders' name
			if (folders < maxDisplayedInLabel) {
				newLabel.append(fileInfo.fileName()).append(", ");
				folders++;
			}
			externalLocations.append(fileInfo.absoluteFilePath());
			onlyFiles = false;
		} else if (fileInfo.isFile()){
			externalLocations.append(fileInfo.absoluteFilePath());
			onlyFiles = onlyFiles && true;
		}
	}
	if (newLabel.length() > 2) {
		newLabel = newLabel.left(newLabel.length() - 2);
		if (folders >= maxDisplayedInLabel) {
			newLabel.append(", ... ");
		}
		labelHowToProceed->setText(labelHowToProceed->text().arg(newLabel));
	}
	return onlyFiles;
}

void DragDropDialog::addExternalFoldersToLibrary()
{
	if (checkBoxRememberChoice->isChecked()) {
		SettingsPrivate::instance()->setDragDropAction(SettingsPrivate::DD_AddToLibrary);
	}
	QList<QDir> dirs;
	for (QString dir : externalLocations) {
		dirs << dir;
	}
	emit aboutToAddExtFoldersToLibrary(dirs);
	this->accept();
}

void DragDropDialog::addExternalFoldersToPlaylist()
{
	if (checkBoxRememberChoice->isChecked()) {
		SettingsPrivate::instance()->setDragDropAction(SettingsPrivate::DD_AddToPlaylist);
	}
	QList<QDir> dirs;
	for (QString dir : externalLocations) {
		dirs << dir;
	}
	emit aboutToAddExtFoldersToPlaylist(dirs);
	this->accept();
}
