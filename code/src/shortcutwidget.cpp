#include "shortcutwidget.h"
#include "settings.h"

#include "QHBoxLayout"

#include <QtDebug>

ShortcutWidget::ShortcutWidget(QWidget *parent) :
	QWidget(parent)
{
	QHBoxLayout *layout = new QHBoxLayout(this);
	comboBox = new QComboBox(this);
	plusLabel = new QLabel(this);
	lineEdit = new ShortcutLineEdit(this);

	comboBox->addItem(QString(), 0);
	comboBox->addItem(tr("Ctrl"), Qt::CTRL);
	comboBox->addItem(tr("Shift"), Qt::SHIFT);
	comboBox->addItem(tr("Alt"), Qt::ALT);

	plusLabel->setMinimumWidth(10);

	layout->addWidget(comboBox);
	layout->addWidget(plusLabel);
	layout->addWidget(lineEdit);

	setLayout(layout);

	connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(showPlusLabel(int)));
	connect(lineEdit, SIGNAL(editingFinished()), this, SLOT(createKeySequence()));
}

void ShortcutWidget::setObjectName(const QString &name)
{
	QKeySequence shortcut = Settings::getInstance()->shortcut(name.left(name.size() - QString("ShortcutWidget").size()));
	if (!shortcut.isEmpty()) {
		QStringList keys = shortcut.toString().split('+');
		// If we have a modifier like Ctrl
		/// bug translation
		if (keys.size() > 1) {
			int index = comboBox->findText(tr(keys.first().toStdString().data()));
			comboBox->setCurrentIndex(index);
			lineEdit->setText(tr(keys.last().toStdString().data()));
		} else {
			QString translated = tr(shortcut.toString().toStdString().data());
			lineEdit->setText(translated);
		}
	}
	QObject::setObjectName(name);
}

/** Create a QKeySequence from a combobox and a label. */
void ShortcutWidget::createKeySequence()
{
	if (!lineEdit->text().isEmpty()) {
		int modifier = comboBox->itemData(comboBox->currentIndex()).toInt();
		emit shortcutChanged(objectName().remove("ShortcutWidget"), QKeySequence(modifier + lineEdit->key()));
	}
}

/** Add the '+' symbol when a modifier key is selected. */
void ShortcutWidget::showPlusLabel(int i)
{
	if (i == 0) {
		plusLabel->clear();
	} else {
		plusLabel->setText("+");
	}
	if (!lineEdit->text().isEmpty()) {
		emit createKeySequence();
	}
}
