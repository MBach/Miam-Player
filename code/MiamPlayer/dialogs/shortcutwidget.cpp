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
	lineEdit->setClearButtonEnabled(true);

	comboBox->addItem(QString(), 0);
	comboBox->addItem(tr("Ctrl"), Qt::CTRL);
	comboBox->addItem(tr("Shift"), Qt::SHIFT);
	comboBox->addItem(tr("Alt"), Qt::ALT);
	comboBox->addItem(tr("Ctrl + Shift"), Qt::CTRL | Qt::SHIFT);

	plusLabel->setMinimumWidth(10);

	layout->addWidget(comboBox);
	layout->addWidget(plusLabel);
	layout->addWidget(lineEdit);
	layout->setSizeConstraint(QLayout::SetMinAndMaxSize);
	setLayout(layout);

	connect(comboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ShortcutWidget::showPlusLabel);
	connect(lineEdit, &QLineEdit::editingFinished, this, &ShortcutWidget::createKeySequence);
	connect(lineEdit, &QLineEdit::clear, this, &ShortcutWidget::deleteKeySequence);
}

void ShortcutWidget::setObjectName(const QString &name)
{
	int shortcut = Settings::getInstance()->shortcut(name);
	if (shortcut > 0) {
		QKeySequence keySequence(shortcut);
		QStringList keys = keySequence.toString().split('+');
		// If we have a modifier like 'Ctrl'
		if (keys.size() > 1) {
			int index = comboBox->findText(keys.first());
			comboBox->setCurrentIndex(index);

			// Removes the modifier
			shortcut -= comboBox->itemData(index).toInt();
			keySequence = QKeySequence(shortcut);
		}
		lineEdit->setKey(shortcut);
	}
	QObject::setObjectName(name);
}

/** Create a QKeySequence from a combobox and a label. */
void ShortcutWidget::createKeySequence()
{
	if (!lineEdit->text().isEmpty()) {
		int modifier = comboBox->itemData(comboBox->currentIndex()).toInt();
		emit shortcutChanged(this, modifier + lineEdit->key());
	}
}

/** Delete a QKeySequence. */
void ShortcutWidget::deleteKeySequence()
{
	comboBox->setCurrentIndex(0);
	lineEdit->setKey(0);
	lineEdit->setStyleSheet(QString());
	emit shortcutChanged(this);
}

/** Add the '+' symbol when a modifier key is selected. */
void ShortcutWidget::showPlusLabel(int i)
{
	if (i == 0) {
		plusLabel->clear();
	} else {
		plusLabel->setText("+");
	}
	lineEdit->setFocus();
	if (!lineEdit->text().isEmpty()) {
		emit createKeySequence();
	}
}
