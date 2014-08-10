#ifndef ABSTRACTSEARCHDIALOG_H
#define ABSTRACTSEARCHDIALOG_H

#include "miamcore_global.h"
#include "interfaces/searchmediaplayerplugin.h"
#include <QCheckBox>
#include <QDialog>
#include <QListWidget>

class MIAMCORE_LIBRARY AbstractSearchDialog : public QWidget
{
	Q_OBJECT
public:
	explicit AbstractSearchDialog(QWidget *parent = 0, Qt::WindowFlags f = 0) : QWidget(parent, f) {}

	virtual ~AbstractSearchDialog() {}

	virtual void addSource(QCheckBox *checkBox) = 0;

	virtual QListWidget * artists() const = 0;

	virtual QListWidget * albums() const = 0;

	virtual QListWidget * tracks() const = 0;

public slots:
	virtual void processResults(SearchMediaPlayerPlugin::Request type, QList<QListWidgetItem*> results) = 0;

signals:
	void aboutToSearch(const QString &text);
};

#endif // ABSTRACTSEARCHDIALOG_H
