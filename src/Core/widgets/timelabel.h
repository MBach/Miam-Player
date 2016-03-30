#ifndef TIMELABEL_H
#define TIMELABEL_H

#include <QLabel>
#include "miamcore_global.h"

/**
 * \brief		Display up to three modes for the length of a track.
 * \author		Matthieu Bachelier
 * \copyright	GNU General Public License v3
 */
class MIAMCORE_LIBRARY TimeLabel : public QLabel
{
	Q_OBJECT
private:
	/** The mode from "mm:ss", "-mm:ss" or "mm:ss / mm:ss" */
	int _mode;

	/** Current time of selected track. */
	qint64 _time;

	/** Total time of selected track. */
	qint64 _total;

public:
	/** Default constructor. */
	explicit TimeLabel(QWidget *parent = nullptr);

	/** Redefined to filter mouse press event. */
	virtual bool eventFilter(QObject *obj, QEvent *event) override;

	virtual QSize minimumSizeHint() const override;

private slots:
	/** Display track length using the selected mode. */
	void display();

public slots:
	void setTime(qint64 time, qint64 total);

signals:
	/** Sent when time has changed to update the label.*/
	void timeChanged();
};

#endif // TIMELABEL_H
