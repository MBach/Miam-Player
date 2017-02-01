#ifndef MATCHINGRECORDSWIDGET_H
#define MATCHINGRECORDSWIDGET_H

#include "mbrelease.h"
#include <QWidget>

#include "ui_matchingrecords.h"

class MatchingRecordsWidget : public QWidget, public Ui::MatchingRecords
{
	Q_OBJECT
private:
	QList<MusicBrainz::Release> _releases;

public:
	explicit MatchingRecordsWidget(QWidget *parent = 0);

public slots:
	void addRelease(const MusicBrainz::Release &release);

	void autoSelectFirstResult();

signals:
	void aboutToHide();
	void releaseChanged(const MusicBrainz::Release &);
};

#endif // MATCHINGRECORDSWIDGET_H
