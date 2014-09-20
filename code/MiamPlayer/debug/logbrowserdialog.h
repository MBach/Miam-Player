#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

class QTextBrowser;
class QPushButton;

class LogBrowserDialog : public QDialog
{
	Q_OBJECT

public:
	LogBrowserDialog(QWidget *parent = 0);
	~LogBrowserDialog() {}

public slots:
	void outputMessage(QtMsgType type, const QString &msg);
	void show();

protected slots:
	void save();

protected:
	virtual void closeEvent(QCloseEvent *e);
	virtual void keyPressEvent(QKeyEvent *e);

	QTextBrowser *browser;
	QPushButton *clearButton;
	QPushButton *saveButton;
};

#endif // DIALOG_H
