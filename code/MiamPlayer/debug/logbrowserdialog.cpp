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
#include <QTableWidget>
#include <QHeaderView>

#include "settings.h"
#include <iostream>

LogBrowserDialog::LogBrowserDialog(QWidget *parent)
	: QDialog(parent)
{
	QVBoxLayout *layout = new QVBoxLayout;
	setLayout(layout);

	_browser = new QTableWidget(0, 2, this);
	_browser->setEditTriggers(QAbstractItemView::NoEditTriggers);
	_browser->horizontalHeader()->setStretchLastSection(true);
	_browser->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	_browser->setHorizontalHeaderLabels(QStringList() << tr("Type") << tr("Message"));
	layout->addWidget(_browser);

	QHBoxLayout *buttonLayout = new QHBoxLayout;
	buttonLayout->setContentsMargins(0, 0, 0, 0);
	layout->addLayout(buttonLayout);

	buttonLayout->addStretch(10);

	_clearButton = new QPushButton(this);
	_clearButton->setText(tr("Clear"));
	buttonLayout->addWidget(_clearButton);

	_saveButton = new QPushButton(this);
	_saveButton->setText(tr("Save output"));
	buttonLayout->addWidget(_saveButton);

	connect(_clearButton, &QPushButton::clicked, [=]() {
		_browser->setRowCount(0);
	});
	connect(_saveButton, &QPushButton::clicked, this, &LogBrowserDialog::save);

	resize(400, 400);
}

void LogBrowserDialog::outputMessage(QtMsgType type, const QString &msg)
{
	int row = _browser->rowCount();
	_browser->insertRow(row);
	switch (type) {
	case QtDebugMsg:
		_browser->setItem(row, 0, new QTableWidgetItem(QIcon(":/debug/hexa"), ""));
		_browser->setItem(row, 1, new QTableWidgetItem(msg));
		break;

	case QtWarningMsg:
		_browser->setItem(row, 0, new QTableWidgetItem(QIcon(":/debug/warning"), ""));
		_browser->setItem(row, 1, new QTableWidgetItem(msg));
		break;

	case QtCriticalMsg:
		_browser->setItem(row, 0, new QTableWidgetItem(QIcon(":/debug/warning"), ""));
		_browser->setItem(row, 1, new QTableWidgetItem(msg));
		break;

	case QtFatalMsg:
		_browser->setItem(row, 0, new QTableWidgetItem(QIcon(":/debug/warning"), ""));
		_browser->setItem(row, 1, new QTableWidgetItem(msg));
		break;
	}
	/// XXX: Hack to have output in Qt Creator on Windows and at the same time in the application
	/// Need to be tested too in Ubuntu
	#ifdef Q_OS_WIN
	std::cerr << msg.toStdString() << std::endl;
	#endif
}

void LogBrowserDialog::show()
{
	this->restoreGeometry(Settings::instance()->value("LogBrowserDialogGeometry").toByteArray());
	QDialog::show();
}

void LogBrowserDialog::save()
{
	QString saveFileName = QFileDialog::getSaveFileName(this, tr("Save Log Output"),
														tr("%1/logfile.txt").arg(QDir::homePath()),
														tr("Text Files (*.txt);;All Files (*)"));

	if (saveFileName.isEmpty()) {
		return;
	}

	QFile file(saveFileName);
	if (!file.open(QIODevice::WriteOnly)) {
		QMessageBox::warning(this, tr("Error"), QString(tr("<nobr>File '%1'<br/>cannot be opened for writing.<br/><br/>"
														   "The log output could <b>not</b> be saved!</nobr>")).arg(saveFileName));
		return;
	}

	QTextStream stream(&file);
	for (int row = 0; row < _browser->rowCount(); row++) {
		if (_browser->item(row, 0)) {
			stream << tr("Warning: ");
		}
		stream << _browser->item(row, 1)->text();
		endl(stream);
	}
	file.close();
}

void LogBrowserDialog::closeEvent(QCloseEvent *e)
{
	Settings::instance()->setValue("LogBrowserDialogGeometry", this->saveGeometry());
	QDialog::closeEvent(e);
}

void LogBrowserDialog::keyPressEvent(QKeyEvent *e)
{
	// ignore all keyboard events
	// protects against accidentally closing of the dialog
	// without asking the user
	e->ignore();
}
