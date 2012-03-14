#include "filesystemmodel.h"

FileSystemModel::FileSystemModel(QObject *parent) :
    QFileSystemModel(parent)
{
	QModelIndex moveUpIndex;
	insertRow(0, moveUpIndex);
}
