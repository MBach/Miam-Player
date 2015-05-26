#include "closeplaylistpopup.h"

#include <QDesktopWidget>
#include <QMessageBox>
#include <QPushButton>

#include <settingsprivate.h>
#include "tabplaylist.h"

#include <QtDebug>

ClosePlaylistPopup::ClosePlaylistPopup(TabPlaylist *p) :
	QDialog(NULL), _index(0),
	deleteButton(new QPushButton(tr("Delete this playlist"), this)),
	replaceButton(new QPushButton(tr("Replace this playlist"), this))
{
	setupUi(this);
	deleteButton->hide();
	replaceButton->hide();
	labelExistingPlaylist->hide();

	connect(buttonBox, &QDialogButtonBox::clicked, this, &ClosePlaylistPopup::execActionFromClosePopup);
	connect(checkBoxRememberChoice, &QCheckBox::toggled, buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::setDisabled);
}

void ClosePlaylistPopup::setDeleteMode(bool del)
{
	labelExistingPlaylist->setVisible(del);
	labelNewPlaylist->setVisible(!del);
	checkBoxRememberChoice->setVisible(!del);
	if (del) {
		buttonBox->removeButton(replaceButton);
		buttonBox->setStandardButtons(QDialogButtonBox::Discard | QDialogButtonBox::Cancel);
		buttonBox->addButton(deleteButton, QDialogButtonBox::AcceptRole);
	} else {
		buttonBox->setStandardButtons(QDialogButtonBox::Save | QDialogButtonBox::Discard | QDialogButtonBox::Cancel);
		buttonBox->removeButton(deleteButton);
		buttonBox->removeButton(replaceButton);
	}
}

void ClosePlaylistPopup::setOverwriteMode(bool overwrite)
{
	labelExistingPlaylist->setVisible(overwrite);
	labelNewPlaylist->setVisible(!overwrite);
	checkBoxRememberChoice->setVisible(!overwrite);
	if (overwrite) {
		buttonBox->removeButton(deleteButton);
		buttonBox->setStandardButtons(QDialogButtonBox::Discard | QDialogButtonBox::Cancel);
		buttonBox->addButton(replaceButton, QDialogButtonBox::AcceptRole);
	} else {
		buttonBox->setStandardButtons(QDialogButtonBox::Save | QDialogButtonBox::Discard | QDialogButtonBox::Cancel);
		buttonBox->removeButton(replaceButton);
		buttonBox->removeButton(deleteButton);
	}
}

void ClosePlaylistPopup::setVisible(bool visible)
{
	if (checkBoxRememberChoice->isChecked()) {
		checkBoxRememberChoice->toggle();
	}
	if (!visible) {
		setOverwriteMode(visible);
	} else {
		int w = qApp->desktop()->screenGeometry().width() / 2;
		int h = qApp->desktop()->screenGeometry().height() / 2;
		this->move(w - frameGeometry().width() / 2, h - frameGeometry().height() / 2);
	}
	QDialog::setVisible(visible);
}

void ClosePlaylistPopup::execActionFromClosePopup(QAbstractButton *action)
{
   if (action == replaceButton) {
	   emit aboutToSavePlaylist(index(), true);
   } else if (action == deleteButton){
	   emit aboutToDeletePlaylist(index());
   } else {
	   // Standard enumeration
	   switch(buttonBox->standardButton(action)) {
	   case QDialogButtonBox::Save:
		   if (checkBoxRememberChoice->isChecked()) {
			   SettingsPrivate::instance()->setPlaybackCloseAction(SettingsPrivate::PL_SaveOnClose);
		   }
		   emit aboutToSavePlaylist(index(), false);
		   break;
	   case QDialogButtonBox::Discard:
		   if (checkBoxRememberChoice->isChecked()) {
			   SettingsPrivate::instance()->setPlaybackCloseAction(SettingsPrivate::PL_DiscardOnClose);
		   }
		   emit aboutToRemoveTab(index());
		   break;
	   default:
		   break;
	   }
   }
  }
