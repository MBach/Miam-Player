#ifndef CUSTOMIZETHEMEDIALOG_H
#define CUSTOMIZETHEMEDIALOG_H

#include "colordialog.h"
#include <QDialog>

#include "ui_customizetheme.h"
#include "reflector.h"

#include <QPropertyAnimation>
#include <QTimer>

/**
 * \brief		The CustomizeThemeDialog class is a very important class. It is designed to help one to customize theme of Miam-Player.
 * \details		Almost everything can be customize: buttons, volume bar, fonts, colors, library and tabs. Lots of efforts have been made
 *		to apply these effects at runtime, in real time, without classic "Apply button". Also, no buttons to reset defaults are present. It is a choice
 *		and it won't be changed. Except for rare cases (colors) because it can be absolutely awful.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class CustomizeThemeDialog : public QDialog, public Ui::CustomizeThemeDialog
{
	Q_OBJECT

private:
	Reflector *_targetedColor;

	/** Used to make this dialog transparent to have a nice fading effect. */
	QPropertyAnimation *_animation;

	/** Duration of the fading effect. */
	QTimer *_timer;

public:
	CustomizeThemeDialog(QWidget *parent = nullptr);

	inline Reflector* targetedColor() const { return _targetedColor; }

private:
	void animate(qreal startValue, qreal stopValue);

	void fade();

	/** Load theme at startup. */
	void loadTheme();

	void setupActions();

protected:
	/** Automatically centers the parent window when closing this dialog. */
	virtual void closeEvent(QCloseEvent *e) override;

	virtual void showEvent(QShowEvent * event) override;

public slots:
	/** Redefined to initialize favorites from settings. */
	virtual int exec() override;

private slots:
	void openChooseIconDialog();

	/** Changes the current theme and updates this dialog too. */
	void setThemeNameAndDialogButtons(QString);

	/** Shows a color dialog and hides this dialog temporarily.
	 * Also, reorder the mainWindow and the color dialog to avoid overlapping, if possible. */
	void showColorDialog();

	void toggleCustomColors(bool b);

	void toggleCustomTextColors(bool b);
};

#endif // CUSTOMIZETHEMEDIALOG_H
