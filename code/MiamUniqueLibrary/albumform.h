#ifndef ALBUMFORM_H
#define ALBUMFORM_H

#include <QWidget>

#include "ui_templateAlbum.h"

class AlbumForm : public QWidget, public Ui::AlbumForm
{
	Q_OBJECT
public:
	explicit AlbumForm(QWidget *parent = 0);

	void setArtist(const QString &artist);
	void setAlbum(const QString &album, int year);
	void setCover(const QString &coverPath);
	void setDiscNumber(int disc);
	void appendTrack(const QString &track);

signals:

public slots:

};

#endif // ALBUMFORM_H
