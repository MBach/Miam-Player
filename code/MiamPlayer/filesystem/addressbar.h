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
	QStack<QDir> _hiddenFolders;

	AddressBarButton *_lastHighlightedButton;

public:
	explicit AddressBar(QWidget *parent = 0);

	/** Called by the popup menu when one is moving the mouse cursor. */
	void findAndHighlightButton(const QPoint &p);

protected:
	virtual void paintEvent(QPaintEvent *);

	virtual void resizeEvent(QResizeEvent *event);

private:
	/** Delete subdirectories located after the arrow button. */
	void clear();

	/** Create a special root arrow button.*/
	void createRoot();

	/** Append a button to the address bar to navigate through the filesystem. */
	int createSubDirButtons(const QDir &path);

public slots:
	/** Init with an absolute path. Also used as a callback to a view. */
	void init(const QDir &initDir);

private slots:
	/** Show a popup menu with the content of the selected directory. */
	void showSubDirMenu(AddressBarButton *button);

	/** Show logical drives (on Windows) or root item (on Unix). Also, when the path is too long, first folders are sent to this submenu. */
	void showDrivesAndPreviousFolders();

signals:
	void aboutToChangePath(const QDir &);
};

#endif // ADDRESSBAR_H
