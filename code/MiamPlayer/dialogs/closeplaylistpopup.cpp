#include "closeplaylistpopup.h"

#include <QDesktopWidget>
#include <QMessageBox>
#include <QPushButton>

#include <QtDebug>

ClosePlaylistPopup::ClosePlaylistPopup(QWidget *parent) :
	QDialog(parent), _index(0),
	deleteButton(new QPushButton(tr("Delete this playlist"), this)),
	replaceButton(new QPushButton(tr("Replace this playlist"), this))
{
	setupUi(this);
	deleteButton->hide();
	replaceButton->hide();
	labelExistingPlaylist->hide();

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
