#ifndef JUMPTOWIDGET_H
#define JUMPTOWIDGET_H

#include <QWidget>

class LibraryTreeView;

class JumpToWidget : public QWidget
{
	Q_OBJECT
private:
	LibraryTreeView *_libraryTreeView;

public:
	explicit JumpToWidget(LibraryTreeView *treeView);

	bool eventFilter(QObject *obj, QEvent *event);

	virtual QSize sizeHint() const;

protected:
	void paintEvent(QPaintEvent *event);
};

#endif // JUMPTOWIDGET_H
