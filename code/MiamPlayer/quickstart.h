#ifndef QUICKSTARTTABLEWIDGET_H
#define QUICKSTARTTABLEWIDGET_H

#include <QFileInfo>
#include <QWidget>

#include <quickstartsearchengine.h>

#include "ui_quickstart.h"

class QuickStart : public QWidget, public Ui::QuickStart
{
	Q_OBJECT
private:
	static const QList<int> ratios;

	int _totalMusicFiles;

	QThread *_worker;
	QuickStartSearchEngine *_qsse;

public:
	explicit QuickStart(QWidget *parent = 0);

	virtual bool eventFilter(QObject *, QEvent *e);

	/** The first time the player is launched, this function will scan for multimedia files. */
	void searchMultimediaFiles();

private slots:
	void checkRow(int row, int);

public slots:
	/** Insert above other rows a new one with a Master checkbox to select/unselect all. */
	void insertFirstRow();

	/** Insert a row with a checkbox with folder's name and the number of files in this folder. */
	void insertRow(const QFileInfo &, const int &);
};

#endif // QUICKSTARTTABLEWIDGET_H
