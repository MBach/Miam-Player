#include "albumform.h"

AlbumForm::AlbumForm(QWidget *parent) :
	QWidget(parent)
{
	setupUi(this);
}

void AlbumForm::setArtist(const QString &artist)
{
	artistPushButton->setText(artist);
}

void AlbumForm::setAlbum(const QString &album, int year)
{
	QString text = album + " [" + QString::number(year) + "]";
	albumPushButton->setText(text);
}

#include <QImageReader>

void AlbumForm::setCover(const QString &coverPath)
{
	static QImageReader imageReader;
	/*if (item->icon().isNull() && !file.isEmpty()) {
		FileHelper fh(file);
		QFileInfo f(file);
		// If it's an inner cover
		if (FileHelper::suffixes().contains(f.suffix())) {
			Cover *cover = fh.extractCover();
			QPixmap p;
			p.loadFromData(cover->byteArray(), cover->format());
			item->setIcon(QIcon(p));
		} else {
			imageReader.setFileName(QDir::fromNativeSeparators(file));
			imageReader.setScaledSize(QSize(Settings::getInstance()->coverSize(), Settings::getInstance()->coverSize()));
			item->setIcon(QIcon(QPixmap::fromImage(imageReader.read())));
		}
	}*/
}

void AlbumForm::setDiscNumber(int discNumber)
{
	if (discNumber == 0) {
		discPushButton->hide();
	} else {
		discPushButton->setText(QString::number(discNumber));
	}
}

void AlbumForm::appendTrack(const QString &track)
{
	tracksWidget->addItem(track);
}
