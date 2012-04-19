#ifndef SHORTCUTWIDGET_H
#define SHORTCUTWIDGET_H

#include <QWidget>

#include <QComboBox>
#include <QKeyEvent>
#include <QLabel>

#include "shortcutlineedit.h"

class ShortcutWidget : public QWidget
{
	Q_OBJECT
private:
	QComboBox *comboBox;
	QLabel *plusLabel;
	ShortcutLineEdit *lineEdit;

public:
	ShortcutWidget(QWidget *parent = 0);

	void setObjectName(const QString &name);
	
signals:
	void shortcutChanged(QString, QKeySequence);

private slots:
	/** Create a QKeySequence from a combobox and a label. */
	void createKeySequence();

	/** Add the '+' symbol when a modifier key is selected. */
	void showPlusLabel(int i);
};

#endif // SHORTCUTWIDGET_H
