#include "tagconverter.h"

#include <QtDebug>

TagConverter::TagConverter(QWidget *parent)	:
	QDialog(parent, Qt::Popup)
{
	setupUi(this);

	foreach (QToolButton *toolButton, findChildren<QToolButton*>()) {
		connect(toolButton, &QToolButton::clicked, this, [=]() {
			tagPattern->addTag(toolButton->text());
		});
	}
}
