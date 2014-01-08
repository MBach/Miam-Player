#include "closeplaylistpopup.h"

#include <QPushButton>

ClosePlaylistPopup::ClosePlaylistPopup(QWidget *parent) :
	QDialog(parent), _index(0)
{
	setupUi(this);

	connect(checkBoxRememberChoice, &QCheckBox::toggled, buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::setDisabled);
}
