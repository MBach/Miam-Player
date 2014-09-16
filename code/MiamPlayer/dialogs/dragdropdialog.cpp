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

/** Is it necessary to redefined this from the UI class just for this init label? */
void DragDropDialog::retranslateUi(DragDropDialog *dialog)
{
	labelHowToProceed->setText("What would you like to do with %1?");
	Ui::DragDropDialog::retranslateUi(dialog);
}

bool DragDropDialog::setMimeData(const QMimeData *mimeData)
{
	_externalLocations.clear();
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
			_externalLocations.append(fileInfo.absoluteFilePath());
			onlyFiles = false;
		} else if (fileInfo.isFile()){
			_externalLocations.append(fileInfo.absoluteFilePath());
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
		SettingsPrivate::getInstance()->setDragDropAction(SettingsPrivate::DD_AddToLibrary);
	}
	QList<QDir> dirs;
	foreach (QString dir, _externalLocations) {
		dirs << dir;
	}
	emit aboutToAddExtFoldersToLibrary(dirs);
	this->accept();
}

void DragDropDialog::addExternalFoldersToPlaylist()
{
	if (checkBoxRememberChoice->isChecked()) {
		SettingsPrivate::getInstance()->setDragDropAction(SettingsPrivate::DD_AddToPlaylist);
	}
	QList<QDir> dirs;
	foreach (QString dir, _externalLocations) {
		dirs << dir;
	}
	emit aboutToAddExtFoldersToPlaylist(dirs);
	this->accept();
}
