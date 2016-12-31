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
}

bool DragDropDialog::setMimeData(const QMimeData *mimeData)
{
	if (!mimeData->hasUrls()) {
		return false;
	}
	bool onlyFiles = true;
	QList<QUrl> urlList = mimeData->urls();
	QStringList folders;

	for (QUrl url : urlList) {
		QFileInfo fileInfo = url.toLocalFile();
		if (fileInfo.isDir()) {
			folders << fileInfo.fileName();
			_externalLocations.append(QDir::toNativeSeparators(fileInfo.absoluteFilePath()));
			onlyFiles = false;
		} else if (fileInfo.isFile()) {
			if (FileHelper::suffixes(FileHelper::ET_Playlist).contains(fileInfo.suffix())) {
				_playlistLocations.append(fileInfo.absoluteFilePath());
			} else {
				_externalLocations.append(fileInfo.absoluteFilePath());
			}
		}
	}

	// Builds the label as a concatenation of folders' name
	QString newLabel;
	if (folders.size() > 3) {
		for (int i = 0; i < 3; i++) {
			newLabel.append(folders.at(i)).append(", ");
		}
		newLabel.append("…");
	} else {
		newLabel = folders.join(", ");
	}
	labelHowToProceed->setText(labelHowToProceed->text().arg(newLabel));
	return onlyFiles;
}

void DragDropDialog::addExternalFoldersToLibrary()
{
	if (checkBoxRememberChoice->isChecked()) {
		SettingsPrivate::instance()->setDragDropAction(SettingsPrivate::DD_AddToLibrary);
	}
	QList<QDir> dirs;
	for (QString dir : _externalLocations) {
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
	for (QString dir : _externalLocations) {
		dirs << dir;
	}
	emit aboutToAddExtFoldersToPlaylist(dirs);
	this->accept();
}
