#include "logbrowserdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextBrowser>
#include <QPushButton>
#include <QFileDialog>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QCloseEvent>
#include <QKeyEvent>

#include "settings.h"

LogBrowserDialog::LogBrowserDialog(QWidget *parent)
	: QDialog(parent)
{
	QVBoxLayout *layout = new QVBoxLayout;
	setLayout(layout);

	browser = new QTextBrowser(this);
	layout->addWidget(browser);

	QHBoxLayout *buttonLayout = new QHBoxLayout;
	buttonLayout->setContentsMargins(0, 0, 0, 0);
	layout->addLayout(buttonLayout);

	buttonLayout->addStretch(10);

	clearButton = new QPushButton(this);
	clearButton->setText("clear");
	buttonLayout->addWidget(clearButton);

	saveButton = new QPushButton(this);
	saveButton->setText("save output");
	buttonLayout->addWidget(saveButton);

	connect(clearButton, &QPushButton::clicked, browser, &QTextBrowser::clear);
	connect(saveButton, &QPushButton::clicked, this, &LogBrowserDialog::save);

	resize(400, 400);
}

void LogBrowserDialog::outputMessage(QtMsgType type, const QString &msg)
{
	switch (type) {
	case QtDebugMsg:
		browser->append(msg);
		break;

	case QtWarningMsg:
		browser->append(tr("-- WARNING: %1").arg(msg));
		break;

	case QtCriticalMsg:
		browser->append(tr("-- CRITICAL: %1").arg(msg));
		break;

	case QtFatalMsg:
		browser->append(tr("-- FATAL: %1").arg(msg));
		break;
	}
}

void LogBrowserDialog::show()
{
	this->restoreGeometry(Settings::getInstance()->value("LogBrowserDialogGeometry").toByteArray());
	QDialog::show();
}

void LogBrowserDialog::save()
{
	QString saveFileName = QFileDialog::getSaveFileName(
				this,
				tr("Save Log Output"),
				tr("%1/logfile.txt").arg(QDir::homePath()),
				tr("Text Files (*.txt);;All Files (*)")
				);

	if(saveFileName.isEmpty())
		return;

	QFile file(saveFileName);
	if(!file.open(QIODevice::WriteOnly)) {
		QMessageBox::warning(
					this,
					tr("Error"),
					QString(tr("<nobr>File '%1'<br/>cannot be opened for writing.<br/><br/>"
							   "The log output could <b>not</b> be saved!</nobr>"))
					.arg(saveFileName));
		return;
	}

	QTextStream stream(&file);
	stream << browser->toPlainText();
	file.close();
}

void LogBrowserDialog::closeEvent(QCloseEvent *e)
{
	Settings::getInstance()->setValue("LogBrowserDialogGeometry", this->saveGeometry());
	QDialog::closeEvent(e);
}

void LogBrowserDialog::keyPressEvent(QKeyEvent *e)
{
	// ignore all keyboard events
	// protects against accidentally closing of the dialog
	// without asking the user
	e->ignore();
}
