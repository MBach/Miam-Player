#ifndef LOGBROWSER_H
#define LOGBROWSER_H

#include <QObject>

class LogBrowserDialog;

/**
 * \brief		The LogBrowser class shows logs on screen.
 * \details		The wrapper is instantiated in the main function of an application and creates the browser window.
 *				It also acts as an intermediary and converts the const char * based messages from the debug system into QString
 *				based messages. With this trick the debug messages can be sent to the actual browser by the means of signal/slot
 *				connections. This adds basic thread support to the browser.
 * \author      Qt-project.org
 * \copyright   GNU General Public License v3
 */
class LogBrowser : public QObject
{
	Q_OBJECT
public:
	explicit LogBrowser(QObject *parent = 0);
	~LogBrowser();

public slots:
	void outputMessage(QtMsgType type, const QString &msg);

	void show();

signals:
	void sendMessage(QtMsgType type, const QString &msg);

private:
	LogBrowserDialog *browserDialog;
};

#endif // LOGBROWSER_H
