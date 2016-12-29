#include "dragdropdialog.h"

#include <QFileInfo>
#include <QRadioButton>

#include "filehelper.h"
#include "settingsprivate.h"

#include <QtDebug>

DragDropDialog::DragDropDialog(QWidget *parent) :
	QDialog(parent)
{
	setupUi(this);

	connect(toolButtonLibrary, &QToolButton::clicked, this, &DragDropDialog::addExternalFoldersToLibrary);
	connect(toolButtonPlaylist, &QToolButton::clicked, this, &DragDropDialog::addExternalFoldersToPlaylist);

	_originalLabel = labelHowToProceed->text();
}

bool DragDropDialog::setMimeData(const QMimeData *mimeData)
{
	externalLocations.clear();
	playlistLocations.clear();

	if (!mimeData->hasUrls()) {
		return false;
	}
	bool onlyFiles = true;
	QList<QUrl> urlList = mimeData->urls();
	QString newLabel;
	int folders = 0;
	int maxDisplayedInLabel = 3;

	labelHowToProceed->setText(_originalLabel);
	for (QUrl url : urlList) {
		QFileInfo fileInfo = url.toLocalFile();
		if (fileInfo.isDir()) {
			// Builds the label as a concatenation of folders' name
			if (folders < maxDisplayedInLabel) {
				newLabel.append(fileInfo.fileName()).append(", ");
				folders++;
			}
			externalLocations.append(fileInfo.absoluteFilePath());
			onlyFiles = false;
		} else if (fileInfo.isFile()) {
			if (FileHelper::suffixes(FileHelper::ET_Playlist).contains(fileInfo.suffix())) {
				playlistLocations.append(fileInfo.absoluteFilePath());
			} else {
				externalLocations.append(fileInfo.absoluteFilePath());
			}
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
