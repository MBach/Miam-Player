#ifndef ADDRESSBAR_H
#define ADDRESSBAR_H

#include <QDir>
#include <QHBoxLayout>
#include <QMenu>
#include <QWidget>

#include "addressbarbutton.h"

class AddressBar : public QWidget
{
	Q_OBJECT
private:
	QHBoxLayout *hBoxLayout;
	QMenu *menu;

	QString styleSheetDir;
	QString styleSheetArrow;

public:
	explicit AddressBar(QWidget *parent = 0);

	/** Init with an absolute path. */
	void init(const QString &initPath);

private:
	/** Append 2 buttons to the address bar to navigate through the filesystem. */
	void createSubDirButtons(const QDir &path, bool insertFirst = false);

public slots:
	void updateWithNewFolder(const QString &path);

private slots:
	/** Change the selected path then create subdirectories. */
	void appendSubDir(QAction *action);

	/** Delete subdirectories when one clicks in the middle of this address bar. */
	void deleteSubDir(int after = -1);

	/** Show a popup menu with the content of the selected directory. */
	void showSubDirMenu();

signals:
	void pathChanged(const QString &);
};

#endif // ADDRESSBAR_H
