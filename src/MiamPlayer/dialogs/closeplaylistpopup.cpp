#include "closeplaylistpopup.h"

#include <QDesktopWidget>
#include <QMessageBox>
#include <QPushButton>

#include <settingsprivate.h>
#include "playlist.h"

#include <QtDebug>

ClosePlaylistPopup::ClosePlaylistPopup(Playlist *playlist, int index, QWidget *parent) :
	QDialog(parent), _playlist(playlist), _index(index),
	_deleteButton(nullptr),
	_replaceButton(nullptr)
{
	setupUi(this);

	connect(buttonBox, &QDialogButtonBox::clicked, this, &ClosePlaylistPopup::execActionFromClosePopup);
	connect(checkBoxRememberChoice, &QCheckBox::toggled, buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::setDisabled);

	// Delete mode
	if (playlist->mediaPlaylist()->isEmpty()) {
		buttonBox->setStandardButtons(QDialogButtonBox::Discard | QDialogButtonBox::Cancel);
		_deleteButton = new QPushButton(tr("Delete this playlist"), this);
		buttonBox->addButton(_deleteButton, QDialogButtonBox::AcceptRole);
		checkBoxRememberChoice->hide();
	} else if (playlist->hash() != 0 && playlist->isModified()) {
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
		emit aboutToSavePlaylist(true);
	} else if (action == _deleteButton){
		emit aboutToDeletePlaylist(_playlist->id());
	} else {
		// Standard enumeration
		switch(buttonBox->standardButton(action)) {
		case QDialogButtonBox::Save:
			if (checkBoxRememberChoice->isChecked()) {
				SettingsPrivate::instance()->setPlaybackCloseAction(SettingsPrivate::PL_SaveOnClose);
			}
			emit aboutToSavePlaylist(false);
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
			break;
		default:
			break;
		}
	}
}
