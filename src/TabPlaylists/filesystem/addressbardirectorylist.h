#ifndef ADDRESSBARDIRECTORYLIST_H
#define ADDRESSBARDIRECTORYLIST_H

#include <QDir>
#include <QListWidget>

#include "miamtabplaylists_global.hpp"

/**
 * \brief		The AddressBarDirectoryList class is used to mimic the behaviour of Windows' Explorer.
 * \details		When one is clicking in the AddressBar (but not on a button) it shows an edit area. Then, when one is typing a separator
 *				a new popup is being displayed and populated with the content of the subdirectory if it's a valid one.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMTABPLAYLISTS_LIBRARY AddressBarDirectoryList : public QListWidget
{
	Q_OBJECT
private:
	QDir _dir;

public:
	explicit AddressBarDirectoryList(const QDir &dir, QWidget *parent = nullptr);

	void cd(const QString &path);

	void cdUp(const QString &path);

	void filterItems(const QString &path);
};

#endif // ADDRESSBARDIRECTORYLIST_H
