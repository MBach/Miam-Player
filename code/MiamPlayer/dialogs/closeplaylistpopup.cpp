#include "closeplaylistpopup.h"

#include <QDesktopWidget>
#include <QMessageBox>
#include <QPushButton>

#include <settingsprivate.h>

#include <QtDebug>

ClosePlaylistPopup::ClosePlaylistPopup(int index, bool currentPlaylistIsEmpty, bool playlistModified, QWidget *parent) :
	QDialog(parent), _index(index),
	_deleteButton(NULL),
	_replaceButton(NULL)
{
	setupUi(this);

	connect(buttonBox, &QDialogButtonBox::clicked, this, &ClosePlaylistPopup::execActionFromClosePopup);
	connect(checkBoxRememberChoice, &QCheckBox::toggled, buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::setDisabled);

	// Delete mode
	if (currentPlaylistIsEmpty) {
		buttonBox->setStandardButtons(QDialogButtonBox::Discard | QDialogButtonBox::Cancel);
		_deleteButton = new QPushButton(tr("Delete this playlist"), this);
		buttonBox->addButton(_deleteButton, QDialogButtonBox::AcceptRole);
		checkBoxRememberChoice->hide();
	} else if (playlistModified) {
		// Overwrite mode
		labelPlaylist->setText(tr("You're about to close a playlist that you have modified. What would you like to do?"));
		buttonBox->setStandardButtons(QDialogButtonBox::Discard | QDialogButtonBox::Cancel);
		_replaceButton = new QPushButton(tr("Replace this playlist"), this);
		buttonBox->addButton(_replaceButton, QDialogButtonBox::AcceptRole);
	} else {
		// Standard
		labelPlaylist->setText(tr("You're about to close a playlist. What would you like to do?"));
		buttonBox->setStandardButtons(QDialogButtonBox::Save | QDialogButtonBox::Discard | QDialogButtonBox::Cancel);
	}
}

void ClosePlaylistPopup::execActionFromClosePopup(QAbstractButton *action)
{
	if (action == _replaceButton) {
		emit aboutToSavePlaylist(_index, true);
	} else if (action == _deleteButton){
		emit aboutToDeletePlaylist(_index);
	} else {
		// Standard enumeration
		switch(buttonBox->standardButton(action)) {
		case QDialogButtonBox::Save:
			if (checkBoxRememberChoice->isChecked()) {
				SettingsPrivate::instance()->setPlaybackCloseAction(SettingsPrivate::PL_SaveOnClose);
			}
			emit aboutToSavePlaylist(_index, false);
			break;
		case QDialogButtonBox::Discard:
			if (checkBoxRememberChoice->isChecked()) {
				SettingsPrivate::instance()->setPlaybackCloseAction(SettingsPrivate::PL_DiscardOnClose);
			}
			emit aboutToRemoveTab(_index);
			this->close();
			break;
		case QDialogButtonBox::Cancel:
			emit aboutToCancel();
			this->close();
			qDebug() << Q_FUNC_INFO << "Cancel was clicked!";
			break;
		default:
			break;
		}
	}
}
