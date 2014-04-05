#ifndef ADDRESSBAR_H
#define ADDRESSBAR_H

#include <QDir>
#include <QHBoxLayout>
#include <QMenu>
#include <QWidget>

#include "addressbarbutton.h"
#include "addressbarmenu.h"

#include <QLineEdit>
#include <QStack>

class AddressBar : public QWidget
{
	Q_OBJECT
private:
	QHBoxLayout *hBoxLayout;
	AddressBarMenu *menu;
	QLineEdit *_lineEdit;

	QStack<QDir> _hiddenFolders;

public:
	explicit AddressBar(QWidget *parent = 0);

	virtual bool eventFilter(QObject *obj, QEvent *e);

	void findAndHighlightButton(const QPoint &p);

protected:
	virtual void mousePressEvent(QMouseEvent *);

	virtual void paintEvent(QPaintEvent *);

	virtual void resizeEvent(QResizeEvent *event);

private:
	/** Create a special root arrow button.*/
	void createRoot();

	/** Append a button to the address bar to navigate through the filesystem. */
	int createSubDirButtons(const QDir &path);

public slots:
	/** Init with an absolute path. Also used as a callback to a view. */
	void init(const QString &initPath);

private slots:
	/** Delete subdirectories located after the arrow button. */
	void clear();

	/** Show a popup menu with the content of the selected directory. */
	void showSubDirMenu();

	/** Show logical drives (on Windows) or root item (on Unix). Also, when the path is too long, first folders are sent to this submenu. */
	void showDrivesAndPreviousFolders();

	void appendDirToRootButton(const QDir &previousDir);


signals:
	void pathChanged(const QString &);
};

#endif // ADDRESSBAR_H
