#ifndef SHORTCUTWIDGET_H
#define SHORTCUTWIDGET_H

#include <QWidget>

#include <QComboBox>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>

#include "shortcutlineedit.h"

class ShortcutWidget : public QWidget
{
	Q_OBJECT
private:
	QComboBox *comboBox;
	QLabel *plusLabel;
	ShortcutLineEdit *lineEdit;
	QPushButton *reset;
	QIcon *closeButton;

public:
	ShortcutWidget(QWidget *parent = 0);

	/** Redefined to add initialization of this widget. */
	void setObjectName(const QString &name);

	/** Getter for re-translation. */
	inline QComboBox *modifiers() { return comboBox; }

	/** Getter for re-translation. */
	inline ShortcutLineEdit *line() { return lineEdit; }

	inline int key() const { return comboBox->itemData(comboBox->currentIndex()).toInt() + lineEdit->key(); }
	
signals:
	void shortcutChanged(ShortcutWidget *, int=0);

private slots:
	/** Create a QKeySequence from a combobox and a label. */
	void createKeySequence();

	/** Delete a QKeySequence. */
	void deleteKeySequence();

	/** Add the '+' symbol when a modifier key is selected. */
	void showPlusLabel(int i);
};

#endif // SHORTCUTWIDGET_H
