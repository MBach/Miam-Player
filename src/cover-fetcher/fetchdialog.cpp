#include "fetchdialog.h"

#include <model/sqldatabase.h>
#include <filehelper.h>
#include <settings.h>
#include "browseimagewidget.h"

#include <QAbstractButton>
#include <QDir>
#include <QGroupBox>
#include <QListWidget>
#include <QScrollBar>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QStackedWidget>

#include <QtDebug>

FetchDialog::FetchDialog(const QList<CoverArtProvider *> &providers, QWidget *parent)
	: QDialog(parent, Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint)
{
	this->setupUi(this);
	this->setAttribute(Qt::WA_DeleteOnClose);

	// Change UI
	connect(previewSizeSlider, &QSlider::valueChanged, this, &FetchDialog::updateCoverSize);

	Settings *settings = Settings::instance();

	// Setting the value will trigger valueChanged connected upthere on purpose
	previewSizeSlider->setValue(settings->value("providers/previewSizeSliderValue", 0).toInt());
	restoreGeometry(settings->value("providers/geometry").toByteArray());

	// Filter on apply button only
	connect(buttonBox, &QDialogButtonBox::clicked, this, [=](QAbstractButton *button) {
		if (QDialogButtonBox::ApplyRole == buttonBox->buttonRole(button)) {
			this->applyChanges();
		}
		this->deleteLater();
	});

	for (CoverArtProvider *coverArtProvider : providers) {
		connect(coverArtProvider, &CoverArtProvider::aboutToCreateCover, this, &FetchDialog::addCover);
	}
}

FetchDialog::~FetchDialog()
{

}

void FetchDialog::closeEvent(QCloseEvent *)
{
	Settings::instance()->setValue("providers/geometry", saveGeometry());
}

bool FetchDialog::copyCoverToFolder(Cover *cover, QString artistAlbum, QString album)
{
	SqlDatabase db;
	db.open();

	QSqlQuery findFirstTrack(db);
	findFirstTrack.prepare("SELECT uri FROM cache WHERE artistAlbum = ? AND album = ? LIMIT 1");
	findFirstTrack.addBindValue(artistAlbum);
	findFirstTrack.addBindValue(album);
	findFirstTrack.exec();
	if (findFirstTrack.next()) {
		QFileInfo fileInfo(findFirstTrack.record().value(0).toString());
		QString coverName = QString("cover-%1.").arg(qrand()).append(cover->format()).toLower();
		QString absCover = QDir::toNativeSeparators(fileInfo.absoluteDir().absolutePath().append(QDir::separator()).append(coverName));

		// Remove old cover
		bool b = true;
		if (QFile::exists(absCover)) {
			QFile file(absCover);
			b = file.remove();
		}
		if (b) {
			QPixmap p;
			p.loadFromData(cover->byteArray(), cover->format());
			b = p.save(absCover);

			QSqlQuery updateTracks(db);
			updateTracks.prepare("UPDATE cache SET cover = ? WHERE artistAlbum = ? AND album = ?");
			updateTracks.addBindValue(absCover);
			updateTracks.addBindValue(artistAlbum);
			updateTracks.addBindValue(album);
			b = updateTracks.exec();
		}
		return b;
	} else {
		return false;
	}
}

bool FetchDialog::integrateCoverToFile(Cover *cover, QString artistAlbum, QString album)
{
	SqlDatabase db;
	db.open();
	bool b = db.transaction();

	// Before creating the cover, we have to know which file to process
	QSqlQuery findTracks(db);
	findTracks.prepare("SELECT uri FROM cache WHERE artistAlbum = ? AND album = ?");
	findTracks.addBindValue(artistAlbum);
	findTracks.addBindValue(album);
	b = b & findTracks.exec();
	while (findTracks.next()) {
		FileHelper fh(findTracks.record().value(0).toString());
		fh.setCover(cover);
		if (fh.save()) {
			// Cover has been successfully integrated into file
			QSqlQuery updateTrack(db);
			updateTrack.prepare("UPDATE cache SET internalCover = ? WHERE uri = ?");
			updateTrack.addBindValue(fh.fileInfo().absoluteFilePath());
			updateTrack.addBindValue(fh.fileInfo().absoluteFilePath());
			b = b & updateTrack.exec();
		} else {
			qWarning() << "Cover wasn't integrated into file" << fh.fileInfo().absoluteFilePath();
		}
	}
	if (b) {
		db.commit();
	} else {
		db.rollback();
	}
	return b;
}

void FetchDialog::addCover(const QString &album, const QByteArray &coverByteArray)
{
	for (QGroupBox *gb : findChildren<QGroupBox*>()) {
		if (gb->property("album").toString() == album) {

			QPixmap pixmap;
			pixmap.loadFromData(coverByteArray);

			QStackedWidget *list = gb->findChild<QStackedWidget*>();
			QLabel *label = new QLabel;
			label->setPixmap(pixmap);
			label->setScaledContents(true);
			label->setProperty("coverByteArray", coverByteArray);
			list->addWidget(label);
			break;
		}
	}
}

void FetchDialog::applyChanges()
{
	bool integrateCoverToFiles = Settings::instance()->value("providers/integrateCoverToFiles").toBool();

	for (QGroupBox *gb : this->findChildren<QGroupBox*>()) {

		QStackedWidget *remoteCoverList = gb->findChild<QStackedWidget*>();
		QWidget * current = remoteCoverList->currentWidget();
		QLabel *cover = qobject_cast<QLabel*>(current);
		if (cover) {
			Cover *c = new Cover(cover->property("coverByteArray").toByteArray());
			QString artistAlbum = remoteCoverList->property("artistAlbum").toString();
			QString album = remoteCoverList->property("album").toString();

			// Create cover for each file
			if (integrateCoverToFiles) {
				this->integrateCoverToFile(c, artistAlbum, album);
			} else {
				this->copyCoverToFolder(c, artistAlbum, album);
			}
		}
	}
	this->close();
	emit refreshView();
}

void FetchDialog::updateCoverSize(int value)
{
	int size;
	switch (value) {
	case 0:
		size = 250;
		break;
	case 1:
		size = 500;
		break;
	}

	Settings *settings = Settings::instance();
	settings->setValue("providers/coverValueSize", size);
	settings->setValue("providers/previewSizeSliderValue", value);
	previewSizeValue->setText(QString(tr("%1px")).arg(size));
	QSize iconSize(size, size);
	for (QGroupBox *groupBox : this->findChildren<QGroupBox*>()) {

		QLabel *cover = groupBox->findChild<QLabel*>();
		cover->setMinimumSize(iconSize);
		cover->setMaximumSize(iconSize);

		QWidget *container = groupBox->findChild<QWidget*>("container");
		container->setMinimumSize(iconSize);
		container->setMaximumSize(iconSize);

		QStackedWidget *remoteCovers = groupBox->findChild<QStackedWidget*>();
		remoteCovers->setMinimumSize(iconSize);
		remoteCovers->setMaximumSize(iconSize);

		BrowseImageWidget *biw = groupBox->findChild<BrowseImageWidget*>();
		biw->setMinimumSize(iconSize);
		biw->setMaximumSize(iconSize);

		groupBox->setMinimumHeight(size + 10);
		groupBox->setMaximumHeight(size + 10);
	}
}
