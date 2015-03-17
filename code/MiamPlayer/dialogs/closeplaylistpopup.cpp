#include "closeplaylistpopup.h"

#include <QMessageBox>
#include <QPushButton>

#include <QtDebug>

ClosePlaylistPopup::ClosePlaylistPopup(QWidget *parent) :
	QDialog(parent), _index(0), replace(new QPushButton(tr("Replace"), this))
{
	setupUi(this);
	replace->hide();
	labelExistingPlaylist->hide();

	//connect(_replace, &QPushButton::clicked, this, &ClosePlaylistPopup::saveAndOverwritePlaylist);
	connect(checkBoxRememberChoice, &QCheckBox::toggled, buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::setDisabled);
}

void ClosePlaylistPopup::setOverwriteMode(bool overwrite)
{
	labelExistingPlaylist->setVisible(overwrite);
	labelNewPlaylist->setVisible(!overwrite);
	checkBoxRememberChoice->setVisible(!overwrite);
	if (overwrite) {
		buttonBox->setStandardButtons(QDialogButtonBox::Discard | QDialogButtonBox::Cancel);
		buttonBox->addButton(replace, QDialogButtonBox::AcceptRole);
	} else {
		buttonBox->setStandardButtons(QDialogButtonBox::Save | QDialogButtonBox::Discard | QDialogButtonBox::Cancel);
		buttonBox->removeButton(replace);
	}
}

void ClosePlaylistPopup::setVisible(bool visible)
{
	if (checkBoxRememberChoice->isChecked()) {
		checkBoxRememberChoice->toggle();
	}
	if (!visible) {
		setOverwriteMode(visible);
	}
	QDialog::setVisible(visible);
}
