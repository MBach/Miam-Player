#ifndef JUMPTOWIDGET_H
#define JUMPTOWIDGET_H

#include <QMouseEvent>
#include <QWidget>

class LibraryTreeView;

class JumpToWidget : public QWidget
{
	Q_OBJECT
private:
	LibraryTreeView *_libraryTreeView;

	QPoint _pos;

public:
	explicit JumpToWidget(LibraryTreeView *treeView);

	bool eventFilter(QObject *obj, QEvent *event);

	virtual QSize sizeHint() const;

protected:
	void leaveEvent(QEvent *e);

	void mouseMoveEvent(QMouseEvent *e);

	void paintEvent(QPaintEvent *event);
};

#endif // JUMPTOWIDGET_H
