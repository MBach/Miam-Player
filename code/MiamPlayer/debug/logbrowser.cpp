#include "logbrowser.h"

#include <QMetaType>

#include "logbrowserdialog.h"

LogBrowser::LogBrowser(QObject *parent) :
	QObject(parent)
{
	qRegisterMetaType<QtMsgType>("QtMsgType");
	browserDialog = new LogBrowserDialog;
	connect(this, &LogBrowser::sendMessage, browserDialog, &LogBrowserDialog::outputMessage, Qt::QueuedConnection);
}

LogBrowser::~LogBrowser()
{
	delete browserDialog;
}

void LogBrowser::outputMessage(QtMsgType type, const QString &msg)
{
	emit sendMessage(type, msg);
}

void LogBrowser::show()
{
	browserDialog->show();
}
