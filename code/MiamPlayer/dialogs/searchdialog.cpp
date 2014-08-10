#include "searchdialog.h"

#include <QTimer>
#include <QStylePainter>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>

#include "settings.h"
#include "pluginmanager.h"

#include <QtDebug>

/** Constructor. */
SearchDialog::SearchDialog(const SqlDatabase &db, QWidget *parent) :
	AbstractSearchDialog(parent, Qt::Widget), _db(db)
{
	this->setupUi(this);

	/// XXX: factorize this
	// Animates this Dialog
	_timer = new QTimer(this);
	_timer->setInterval(3000);
	_timer->setSingleShot(true);
	_animation = new QPropertyAnimation(this, "windowOpacity");
	_animation->setDuration(400);
	_animation->setTargetObject(this);

	this->setWindowOpacity(0.0);

	connect(closeButton, &QPushButton::clicked, this, &SearchDialog::clear);
	connect(labelSearchMore, &QLabel::linkActivated, this, &SearchDialog::searchMoreResults);

	// Unselect the 2 other lists when one is clicking on one
	foreach (QListWidget *list, findChildren<QListWidget*>()) {
		connect(list, &QListWidget::itemClicked, this, [=]() {
			foreach (QListWidget *otherList, findChildren<QListWidget*>()) {
				if (list != otherList) {
					otherList->clearSelection();
				}
			}
		});
	}

	connect(this, &SearchDialog::aboutToSearch, this, &SearchDialog::search);

	// Update font size
	connect(Settings::getInstance(), &Settings::fontHasChanged, this, [=](Settings::FontFamily ff, const QFont &newFont) {
		if (ff == Settings::FF_Library) {
			foreach (QWidget *o, this->findChildren<QWidget*>()) {
				o->setFont(newFont);
			}
		}
	});

	this->setVisible(false);
	_oldRect = this->geometry();
}

/** Required interface from AbstractSearchDialog class. */
void SearchDialog::addSource(QCheckBox *checkBox)
{
	int i = sources_layout->count(); // Default are: Library Checkbox + HSpacer
	checkBox->setFont(Settings::getInstance()->font(Settings::FF_Library));
	sources_layout->insertWidget(i - 1, checkBox);
}

/** String to look for on every registered search engines. */
void SearchDialog::setSearchExpression(const QString &text)
{
	_artists->clear();
	_albums->clear();
	_tracks->clear();

	emit aboutToSearch(text);
}

/** Redefined from QWidget. */
void SearchDialog::setVisible(bool visible)
{
	if (visible) {
		QWidget::setVisible(true);
		if (!_timer->isActive()) {
			this->animate(0.0, 1.0);
		}
		_timer->start();
	} else {
		QWidget::setVisible(false);
		this->setWindowOpacity(0.0);
		_timer->stop();
	}
}

/** Custom rendering. */
void SearchDialog::paintEvent(QPaintEvent *)
{
	QStylePainter p(this);
	QPalette palette = QApplication::palette();
	p.setPen(palette.mid().color());
	p.setBrush(palette.base());
	p.drawRect(rect().adjusted(0, 0, -1, -1));
	p.setPen(palette.midlight().color());
	p.drawLine(1, labelSearchMore->height() - 1, rect().width() - 2, labelSearchMore->height() - 1);
	int y = rect().y() + rect().height() - aggregated->height();
	p.drawLine(39, rect().y() + labelSearchMore->height(), 39, y);
	p.drawLine(1, y, rect().width() - 2, y);
}

/** Process results sent back from various search engines (local, remote). */
void SearchDialog::processResults(SearchMediaPlayerPlugin::Request type, QList<QListWidgetItem*> results)
{
	qDebug() << Q_FUNC_INFO << sender();

	QListWidget *listToProcess = NULL;
	switch (type) {
	case SearchMediaPlayerPlugin::Artist:
		listToProcess = _artists;
		break;
	case SearchMediaPlayerPlugin::Album:
		listToProcess = _albums;
		break;
	case SearchMediaPlayerPlugin::Track:
		listToProcess = _tracks;
		break;
	}
	foreach (QListWidgetItem *item, results) {
		listToProcess->addItem(item);
	}
	listToProcess->sortItems();
	listToProcess->setMaximumHeight(listToProcess->count() * listToProcess->sizeHintForRow(0));
	listToProcess->setMinimumHeight(listToProcess->count() * listToProcess->sizeHintForRow(0));
	qDebug() << "number of items" << listToProcess->count();

	int h = qMax(_artists->count() * _artists->sizeHintForRow(0), iconArtists->height()) +
			qMax(_albums->count() * _albums->sizeHintForRow(0), iconAlbums->height()) +
			qMax(_tracks->count() * _tracks->sizeHintForRow(0), iconTracks->height());
	h += labelSearchMore->height() + aggregated->height() + 3;
	int minW = qMax(iconArtists->width() + _artists->sizeHintForColumn(0), 400);
	this->resize(minW, h);

	qDebug() << "sizeHintForRow" << _albums->sizeHintForRow(0);
}

/// XXX: factorize code
void SearchDialog::animate(qreal startValue, qreal stopValue)
{
	_animation->setStartValue(startValue);
	_animation->setEndValue(stopValue);
	_animation->start();
}

void SearchDialog::clear()
{
	this->close();
	this->setGeometry(_oldRect);
}

/** Local search for matching expressions. */
void SearchDialog::search(const QString &text)
{
	qDebug() << Q_FUNC_INFO;
	if (!checkBoxLibrary->isChecked()) {
		return;
	}

	if (!_db.isOpen()) {
		_db.open();
	}

	QSqlQuery qSearchForArtists(_db);
	qSearchForArtists.prepare("SELECT DISTINCT artist FROM tracks WHERE artist like :t LIMIT 5");
	qSearchForArtists.bindValue(":t", "%" + text + "%");
	if (qSearchForArtists.exec()) {
		while (qSearchForArtists.next()) {
			//qDebug() << qSearchForArtists.record().value(0);
			new QListWidgetItem(qSearchForArtists.record().value(0).toString(), _artists);
		}
	}
	_artists->setMinimumHeight(_artists->count() * _artists->sizeHintForRow(0));

	QSqlQuery qSearchForAlbums(_db);
	qSearchForAlbums.prepare("SELECT DISTINCT album, COALESCE(artistAlbum, artist) FROM tracks WHERE album like :t LIMIT 5");
	qSearchForAlbums.bindValue(":t", "%" + text + "%");
	if (qSearchForAlbums.exec()) {
		QList<QListWidgetItem*> albumList;
		while (qSearchForAlbums.next()) {
			//qDebug() << qSearchForAlbums.record().value(0);
			QListWidgetItem *album = new QListWidgetItem(qSearchForAlbums.record().value(0).toString() + " – " + qSearchForAlbums.record().value(1).toString());
			albumList.append(album);
		}
		this->processResults(SearchMediaPlayerPlugin::Album, albumList);
	}

	QSqlQuery qSearchForTracks(_db);
	qSearchForTracks.prepare("SELECT DISTINCT title, COALESCE(artistAlbum, artist) FROM tracks WHERE title like :t LIMIT 5");
	qSearchForTracks.bindValue(":t", "%" + text + "%");
	if (qSearchForTracks.exec()) {
		while (qSearchForTracks.next()) {
			//qDebug() << qSearchForTracks.record().value(0);
			_tracks->addItem(qSearchForTracks.record().value(0).toString() + " – " + qSearchForTracks.record().value(1).toString());
		}
	}
	_tracks->setMinimumHeight(_tracks->count() * _tracks->sizeHintForRow(0));

	_db.close();
}

/** Expand this dialog to all available space. */
void SearchDialog::searchMoreResults(const QString &link)
{
	if (link == "#") {
		QWidget *p = this;
		QRect mainWindowRect;
		while (p->parentWidget() != NULL) {
			p = p->parentWidget();
			if (p->parentWidget() == NULL) {
				mainWindowRect = p->rect();
			}
		}
		this->move(QPoint(0, 0));
		this->resize(mainWindowRect.size());
	}
}
