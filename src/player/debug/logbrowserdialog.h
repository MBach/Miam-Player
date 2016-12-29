#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

/// Forward declarations
class QTextBrowser;
class QPushButton;
class QTableWidget;

/**
 * \brief		The LogBrowserDialog class is a popup which converts debug strings.
 * \author      Qt-project.org
 * \copyright   GNU General Public License v3
 */
class LogBrowserDialog : public QDialog
{
	Q_OBJECT
private:
	QTableWidget *_browser;
	QPushButton *_clearButton;
	QPushButton *_saveButton;

public:
	LogBrowserDialog(QWidget *parent = nullptr);
	~LogBrowserDialog() {}

protected:
	virtual void closeEvent(QCloseEvent *e) override;
	virtual void keyPressEvent(QKeyEvent *e) override;

public slots:
	void outputMessage(QtMsgType type, const QString &msg);
	void show();

protected slots:
	void save();

};

#endif // DIALOG_H
