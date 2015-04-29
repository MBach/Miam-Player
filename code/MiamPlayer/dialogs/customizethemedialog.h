#ifndef CUSTOMIZETHEMEDIALOG_H
#define CUSTOMIZETHEMEDIALOG_H

#include "colordialog.h"
#include <QDialog>

#include "ui_customizetheme.h"
#include "reflector.h"

#include <QPropertyAnimation>
#include <QTimer>

class MainWindow;

/**
 * \brief		The CustomizeThemeDialog class is a very important class. It is designed to help one to customize theme of Miam-Player.
 * \details		Almost everything can be customize: buttons, volume bar, fonts, colors, library and tabs. Lots of efforts have been made
 * to apply these effects at runtime, in real time, without classic "Apply button". Also, no buttons to reset defaults are present. It is a choice
 * and it won't be changed. Except for rare cases (colors) because it can be absolutely awful.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class CustomizeThemeDialog : public QDialog, public Ui::CustomizeThemeDialog
{
	Q_OBJECT

private:
	MainWindow *mainWindow;

	Reflector *_targetedColor;

	/** Used to make this dialog transparent to have a nice fading effect. */
	QPropertyAnimation *_animation;

	/** Duration of the fading effect. */
	QTimer *_timer;

public:
	CustomizeThemeDialog(QWidget *parent = NULL);

	/** Load theme at startup. */
	void loadTheme();

	inline Reflector* targetedColor() const { return _targetedColor; }

private:
	void fade();
	void setupActions();

	void animate(qreal startValue, qreal stopValue);

protected:
	/** Automatically centers the parent window when closing this dialog. */
	virtual void closeEvent(QCloseEvent *e) override;

	virtual void showEvent(QShowEvent * event) override;

public slots:
	/** Redefined to initialize favorites from settings. */
	virtual int exec() override;

private slots:
	void openChooseIconDialog();

	/** Shows a color dialog and hides this dialog temporarily.
	 * Also, reorder the mainWindow and the color dialog to avoid overlapping, if possible. */
	void showColorDialog();

	void toggleAlternativeBackgroundColor(bool);
	void toggleCustomColors(bool);

	/** Changes the current theme and updates this dialog too. */
	void setThemeNameAndDialogButtons(QString);
};

#endif // CUSTOMIZETHEMEDIALOG_H
