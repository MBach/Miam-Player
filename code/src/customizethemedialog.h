#ifndef CUSTOMIZETHEMEDIALOG_H
#define CUSTOMIZETHEMEDIALOG_H

#include "colordialog.h"
#include <QDialog>

#include "ui_customizetheme.h"
#include "mainwindow.h"
#include "stylesheetupdater.h"
#include "reflector.h"

class CustomizeThemeDialog : public QDialog, public Ui::CustomizeThemeDialog
{
	Q_OBJECT

private:
	MainWindow *mainWindow;

	ColorDialog *colorDialog;

	Reflector *targetedColor;

	StyleSheetUpdater *styleSheetUpdater;

public:
	CustomizeThemeDialog(QWidget *parent);

private:
	void associatePaintableElements();
	void setupActions();

protected:
	/** Automatically centers the parent window when closing this dialog. */
	void closeEvent(QCloseEvent *e);

private:
	/** Load theme at startup. */
	void loadTheme();

public slots:
	/** Redefined to initialize favorites from settings. */
	void open();

private slots:
	void openChooseIconDialog();

	/** Shows a color dialog and hides this dialog temporarily.
	 * Also, reorder the mainWindow and the color dialog to avoid overlapping, if possible. */
	void showColorDialog();

	void changeColor(QColor selectedColor);

	void toggleAlternativeBackgroundColor(bool);

	/** Changes the current theme and updates this dialog too. */
	void setThemeNameAndDialogButtons(QString);

	/** Displays covers or not in the library. */
	void displayCovers(bool);

	/** Displays alphabecical separators or not in the library. */
	void displayAlphabeticalSeparators(bool);

	/** Updates the font family of a specific component. */
	void updateFontFamily(const QFont&);

	/** Updates the font size of a specific component. */
	void updateFontSize(int);
};

#endif // CUSTOMIZETHEMEDIALOG_H
