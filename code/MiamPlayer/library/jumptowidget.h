#ifndef JUMPTOWIDGET_H
#define JUMPTOWIDGET_H

#include <QMouseEvent>
#include <QWidget>

class LibraryTreeView;

/**
 * \brief		The JumpToWidget class displays letters which can be clicked to jump to a particular position in your Library.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
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

signals:
	void displayItemDelegate(bool);
};

#endif // JUMPTOWIDGET_H
