#include "closeplaylistpopup.h"

#include <QPushButton>

ClosePlaylistPopup::ClosePlaylistPopup(QWidget *parent) :
	QDialog(parent)
{
	setupUi(this);

	connect(checkBoxRememberChoice, &QCheckBox::toggled, buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::setDisabled);
}
