#ifndef ABSTRACTSEARCHDIALOG_H
#define ABSTRACTSEARCHDIALOG_H

#include "miamcore_global.h"
#include <QCheckBox>
#include <QDialog>
#include <QListView>
#include <QStandardItem>

typedef QList<QStandardItem*> QStandardItemList;

class MIAMCORE_LIBRARY AbstractSearchDialog : public QWidget
{
	Q_OBJECT
	Q_ENUMS(Request)
public:
	enum Request { Artist = 0,
				   Album = 1,
				   Track = 2};

	explicit AbstractSearchDialog(QWidget *parent = 0, Qt::WindowFlags f = 0) : QWidget(parent, f) {}

	virtual ~AbstractSearchDialog() {}

	virtual void addSource(QCheckBox *checkBox) = 0;

	virtual QListView * artists() const = 0;

	virtual QListView * albums() const = 0;

	virtual QListView * tracks() const = 0;

public slots:
	virtual void processResults(Request type, const QStandardItemList &results) = 0;

signals:
	void aboutToSearch(const QString &text);
};

#endif // ABSTRACTSEARCHDIALOG_H
