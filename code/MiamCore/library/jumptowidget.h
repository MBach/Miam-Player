#ifndef JUMPTOWIDGET_H
#define JUMPTOWIDGET_H

#include <QAbstractItemView>
#include <QMouseEvent>
#include <QWidget>

#include "miamcore_global.h"

/**
 * \brief		The JumpToWidget class displays letters which can be clicked to jump to a particular position in your Library.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY JumpToWidget : public QWidget
{
	Q_OBJECT
private:
	QAbstractItemView *_view;

	QPoint _pos;

	QChar _currentLetter;

public:
	explicit JumpToWidget(QAbstractItemView *treeView);

	virtual bool eventFilter(QObject *obj, QEvent *event);

	void setCurrentLetter(const QChar &currentLetter);

	virtual QSize sizeHint() const;

protected:
	void leaveEvent(QEvent *e);

	void mouseMoveEvent(QMouseEvent *e);

	void paintEvent(QPaintEvent *event);

signals:
	void aboutToScrollTo(const QString &letter);
	void displayItemDelegate(bool);
};

#endif // JUMPTOWIDGET_H
