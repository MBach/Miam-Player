#ifndef ADDRESSBARDIRECTORYLIST_H
#define ADDRESSBARDIRECTORYLIST_H

#include <QDir>
#include <QListWidget>

/**
 * \brief		The AddressBarDirectoryList class
 * \details
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class AddressBarDirectoryList : public QListWidget
{
	Q_OBJECT
private:
	QDir _dir;

public:
	explicit AddressBarDirectoryList(const QDir &dir, QWidget *parent = nullptr);

	void cdUp(const QString &path);

	virtual bool eventFilter(QObject *, QEvent *e) override;

	void filterItems(const QString &path);

protected:
	virtual void focusOutEvent(QFocusEvent *event) override;
};

#endif // ADDRESSBARDIRECTORYLIST_H
