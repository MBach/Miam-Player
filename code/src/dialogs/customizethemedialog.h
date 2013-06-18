#ifndef CUSTOMIZETHEMEDIALOG_H
#define CUSTOMIZETHEMEDIALOG_H

#include "colordialog.h"
#include <QDialog>

#include "ui_customizetheme.h"
#include "mainwindow.h"
#include "stylesheetupdater.h"
#include "reflector.h"

#include <QPropertyAnimation>
#include <QTimer>

class CustomizeThemeDialog : public QDialog, public Ui::CustomizeThemeDialog
{
	Q_OBJECT

private:
	MainWindow *mainWindow;

	ColorDialog *_colorDialog;

	Reflector *_targetedColor;

	StyleSheetUpdater *_styleSheetUpdater;

	/** Used to make this dialog transparent to have a nice fading effect. */
	QPropertyAnimation *_animation;

	/** Duration of the fading effect. */
	QTimer *_timer;

public:
	CustomizeThemeDialog(QWidget *parent);

private:
	void associatePaintableElements();
	void setupActions();

	/** Load theme at startup. */
	void loadTheme();

	void animate(qreal startValue, qreal stopValue);

protected:
	/** Automatically centers the parent window when closing this dialog. */
	void closeEvent(QCloseEvent *e);

	bool eventFilter(QObject *obj, QEvent *event);

public slots:
	/** Redefined to initialize favorites from settings. */
	void open();

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
