#ifndef FILESYSTEMMODEL_H
#define FILESYSTEMMODEL_H

#include <QFileSystemModel>

class FileSystemModel : public QFileSystemModel
{
	Q_OBJECT
public:
	FileSystemModel(QObject *parent = 0);
	
signals:
	
public slots:
	
};

#endif // FILESYSTEMMODEL_H
