#ifndef ADDRESSBAR_H
#define ADDRESSBAR_H

#include <QDir>
#include <QHBoxLayout>
#include <QMenu>
#include <QWidget>

#include "addressbarbutton.h"
#include "addressbarmenu.h"

#include <QLineEdit>

class AddressBar : public QWidget
{
	Q_OBJECT
private:
	QHBoxLayout *hBoxLayout;
	AddressBarMenu *menu;
	QLineEdit *_lineEdit;

public:
	explicit AddressBar(QWidget *parent = 0);

	virtual bool eventFilter(QObject *obj, QEvent *e);

protected:
	virtual void mouseMoveEvent(QMouseEvent *);

	virtual void mousePressEvent(QMouseEvent *);

	virtual void paintEvent(QPaintEvent *);

private:
	/** Create a special root arrow button.*/
	void createRoot();

	/** Append 2 buttons to the address bar to navigate through the filesystem. */
	void createSubDirButtons(const QDir &path, bool insertFirst = false);

	void hideFirstButtons(AddressBarButton *buttonDir);

	void showFirstButtons(AddressBarButton *buttonDir);

public slots:
	/** Init with an absolute path. Also used as a callback to a view. */
	void init(const QString &initPath);

private slots:
	/** Change the selected path then create subdirectories. */
	void appendSubDir(QAction *action);

	/** Delete subdirectories located after the arrow button. */
	void deleteFromArrowFolder(int after);

	/** Delete subdirectories when one clicks in the middle of this address bar. */
	void deleteFromNamedFolder();

	/** Show a popup menu with the content of the selected directory. */
	void showSubDirMenu();

	/** Show logical drives (on Windows) or root item (on Unix). Also, when the path is too long, first folders are sent to this submenu. */
	void showDrivesAndPreviousFolders();

signals:
	void pathChanged(const QString &);
};

#endif // ADDRESSBAR_H
