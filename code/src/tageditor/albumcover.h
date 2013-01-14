#ifndef ALBUMCOVER_H
#define ALBUMCOVER_H

#include <QLabel>
#include <QMenu>

class AlbumCover : public QLabel
{
    Q_OBJECT
private:
	QMenu *imageMenu;

public:
	AlbumCover(QWidget *parent = 0);

	void createPixmapFromFile(const QString &fileName);

protected:
	void contextMenuEvent(QContextMenuEvent *event);
	void dragEnterEvent(QDragEnterEvent *event);
	void dragMoveEvent(QDragMoveEvent *event);
	void dropEvent(QDropEvent *event);

private slots:
	void resetCover();
	void loadCover();

signals:
	void coverHasChanged();
};

#endif // ALBUMCOVER_H
