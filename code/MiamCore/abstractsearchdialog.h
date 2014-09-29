#ifndef ABSTRACTSEARCHDIALOG_H
#define ABSTRACTSEARCHDIALOG_H

#include "miamcore_global.h"
#include <QCheckBox>
#include <QDialog>
#include <QListView>
#include <QStandardItem>

#include "model/remotetrack.h"

typedef QList<QStandardItem*> QStandardItemList;

/**
 * \brief		The AbstractSearchDialog class is a pure virtual class which can be passed to plugins to be extended.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY AbstractSearchDialog : public QWidget
{
	Q_OBJECT
	Q_ENUMS(Request)
	Q_ENUMS(DataType)
public:
	enum Request { Artist = 0,
				   Album = 1,
				   Track = 2};

	enum DataType { DT_Origin = Qt::UserRole + 1,
					DT_Identifier = Qt::UserRole + 2};

	explicit AbstractSearchDialog(QWidget *parent = 0, Qt::WindowFlags f = 0) : QWidget(parent, f) {}

	virtual ~AbstractSearchDialog() {}

	virtual void addSource(QCheckBox *checkBox) = 0;

	virtual QListView * artists() const = 0;

	virtual QListView * albums() const = 0;

	virtual QListView * tracks() const = 0;

public slots:
	virtual void processResults(Request type, const QStandardItemList &results) = 0;

	virtual void aboutToProcessRemoteTracks(const std::list<RemoteTrack> &tracks) = 0;

signals:
	void aboutToSearch(const QString &text);

	// void aboutToSendToCurrentPlaylist();
};

#endif // ABSTRACTSEARCHDIALOG_H
