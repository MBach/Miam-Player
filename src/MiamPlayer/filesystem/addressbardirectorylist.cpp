#include "addressbardirectorylist.h"

#include <QEvent>

#include <QtDebug>

AddressBarDirectoryList::AddressBarDirectoryList(const QDir &dir, QWidget *parent)
	: QListWidget(parent)
	, _dir(dir)
{
	this->setWindowFlags(Qt::FramelessWindowHint);
	QFileInfoList list = _dir.entryInfoList(QStringList(), QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name | QDir::IgnoreCase | QDir::LocaleAware);
	for (QFileInfo f : list) {
		new QListWidgetItem(QDir::toNativeSeparators(f.absoluteFilePath()), this);
	}

	setMinimumHeight(qMin(count(), 16) * this->sizeHintForRow(0) + 4);
	setMaximumHeight(qMin(count(), 16) * this->sizeHintForRow(0) + 4);
	setMinimumWidth(parent->width() - 32);

	this->setFocusPolicy(Qt::ClickFocus);
}

void AddressBarDirectoryList::cd(const QString &path)
{
	if (_dir.cd(path)) {
		this->filterItems(path);
	}
}

void AddressBarDirectoryList::cdUp(const QString &path)
{
	_dir.cdUp();
	this->filterItems(path);
}

void AddressBarDirectoryList::filterItems(const QString &path)
{
	//qDebug() << Q_FUNC_INFO << path;
	QDir d(path);
	if (_dir == d) {
		this->clear();
		QFileInfoList list = _dir.entryInfoList(QStringList(), QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name | QDir::IgnoreCase | QDir::LocaleAware);
		for (QFileInfo f : list) {
			new QListWidgetItem(QDir::toNativeSeparators(f.absoluteFilePath()), this);
		}
	} else {
		QStringList list;
		for (auto item : findItems(path, Qt::MatchStartsWith)) {
			list << item->text();
		}
		this->clear();
		this->addItems(list);
		this->setVisible(!list.isEmpty());
	}
	setMinimumHeight(qMin(count(), 16) * this->sizeHintForRow(0) + 4);
	setMaximumHeight(qMin(count(), 16) * this->sizeHintForRow(0) + 4);
}
