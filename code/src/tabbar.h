#ifndef TABBAR_H
#define TABBAR_H

#include <QLineEdit>
#include <QMouseEvent>
#include <QTabBar>

class TabBar : public QTabBar
{
	Q_OBJECT
private:
	QLineEdit *lineEdit;

public:
	TabBar(QWidget *parent = 0);

	/** Redefined to validate new tab name if the focus is lost. */
	bool eventFilter(QObject *, QEvent *);

protected:
	/** Redefined to display an editable area. */
	void mouseDoubleClickEvent(QMouseEvent *);

	/** Redefined to validate new tab name without pressing return. */
	void mousePressEvent(QMouseEvent *);

private slots:
	/** Rename a tab. */
	void renameTab();
};

#endif // TABBAR_H
