#ifndef CUSTOMIZETHEMEDIALOG_H
#define CUSTOMIZETHEMEDIALOG_H

#include <QDialog>

#include "ui_customizetheme.h"
#include "mainwindow.h"

class CustomizeThemeDialog : public QDialog, public Ui::CustomizeThemeDialog
{
	Q_OBJECT

private:
	MainWindow *mainWindow;

public:
	CustomizeThemeDialog(QWidget *parent);
	
signals:
	void themeChanged();

	void libraryNeedToBeRepaint();

private slots:
	/** Changes the current theme and updates this dialog too. */
	void setThemeNameAndDialogButtons(QString newTheme);
	
public slots:
	void loadTheme();
	void toggleSeparators(bool);
	void updateFontFamily(const QFont&);
	void updateFontSize(int);
};

#endif // CUSTOMIZETHEMEDIALOG_H
