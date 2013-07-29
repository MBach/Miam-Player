#ifndef QUICKSTARTSEARCHENGINE_H
#define QUICKSTARTSEARCHENGINE_H

#include <QFileInfo>

class QuickStartSearchEngine : public QObject
{
	Q_OBJECT
public:
	explicit QuickStartSearchEngine(QObject *parent = 0);
	
public slots:
	void doSearch();

signals:
	void folderScanned(const QFileInfo &, int);
};

#endif // QUICKSTARTSEARCHENGINE_H
