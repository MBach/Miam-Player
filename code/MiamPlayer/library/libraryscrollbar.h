#ifndef LIBRARYSCROLLBAR_H
#define LIBRARYSCROLLBAR_H

#include <QScrollBar>

#include <QModelIndex>
#include <QMouseEvent>

class LibraryTreeView;

class LibraryScrollBar : public QScrollBar
{
	Q_OBJECT

private:
	LibraryTreeView *_libraryTreeView;

	bool _hasNotEmittedYet;

	QWidget *w;

	QModelIndex _letter;

public:
	explicit LibraryScrollBar(LibraryTreeView *parent);

protected:
	void mouseMoveEvent(QMouseEvent *e);

	void mouseReleaseEvent(QMouseEvent *e);

	void paintEvent(QPaintEvent *);

public slots:
	void highlightLetter(const QModelIndex &letter) { _letter = letter; }

private slots:
	void jumpTo(const QString &letter);

signals:
	void displayItemDelegate(bool);
};

#endif // LIBRARYSCROLLBAR_H
