#include "searchdialog.h"

#include <QTimer>
#include <QStandardItemModel>
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
	_artists->setModel(new QStandardItemModel(this));
	_albums->setModel(new QStandardItemModel(this));
	_tracks->setModel(new QStandardItemModel(this));

	// Init map with empty values
	foreach (QListView *list, this->findChildren<QListView*>()) {
		_hiddenItems.insert(list, QList<QStandardItem*>());
		connect(list, &QListView::doubleClicked, this, &SearchDialog::appendSelectedItem);
	}

	_checkBoxLibrary = new QCheckBox(tr("Library"), this);
	_checkBoxLibrary->setChecked(true);
	this->addSource(_checkBoxLibrary);

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

	// Unselect the 2 other lists when one is clicking on another one
	foreach (QListView *list, findChildren<QListView*>()) {
		connect(list->selectionModel(), &QItemSelectionModel::currentRowChanged, this, [=]() {
			foreach (QListView *otherList, findChildren<QListView*>()) {
				if (list != otherList) {
					otherList->selectionModel()->clear();
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
	int i = sources_layout->count(); // Default are: HSpacer
	checkBox->setFont(Settings::getInstance()->font(Settings::FF_Library));
	sources_layout->insertWidget(i - 1, checkBox);

	connect(checkBox, &QCheckBox::toggled, this, [=](bool enabled) {
		foreach (QListView *list, this->findChildren<QListView*>()) {
			QStandardItemModel *m = qobject_cast<QStandardItemModel*>(list->model());
			if (enabled) {
				qDebug() << "Restore hidden items for every list";
				// Restore hidden items for every list
				QList<QStandardItem*> items = _hiddenItems.value(list);
				for (int i = 0; i < items.size(); i++) {
					m->insertRow(i, items.at(i));
				}
				m->sort(0);
			} else {
				qDebug() << "Hide items for every list";
				QStandardItemModel *m = qobject_cast<QStandardItemModel*>(list->model());
				QList<QStandardItem*> items;
				QList<QPersistentModelIndex> indexes;
				for (int i = 0; i < m->rowCount(); i++) {
					QStandardItem *item = m->item(i, 0);
					if (item->data(Qt::UserRole + 1).toString() == checkBox->text()) {
						indexes << m->index(i, 0);
						// Default copy-constructor is protected!
						QStandardItem *copy = new QStandardItem(item->text());
						copy->setData(checkBox->text(), Qt::UserRole + 1);
						items.append(copy);
					}
				}

				foreach (const QPersistentModelIndex &i, indexes) {
					m->removeRow(i.row());
				}

				QList<QStandardItem*> hItems = _hiddenItems.value(list);
				hItems.append(items);
				_hiddenItems.insert(list, hItems);
			}
		}
	});
}

/** String to look for on every registered search engines. */
void SearchDialog::setSearchExpression(const QString &text)
{
	foreach (QListView *view, this->findChildren<QListView*>()) {
		auto model = view->model();
		while (model->rowCount() != 0) {
			model->removeRow(0);
		}
	}
	emit aboutToSearch(text);
}

/** Redefined from QWidget. */
void SearchDialog::setVisible(bool visible)
{
	/*if (visible) {
		this->setWindowOpacity(0.0);
		QWidget::setVisible(true);
		if (!_timer->isActive()) {
			this->animate(0.0, 1.0);
		}
		_timer->start();
	} else {
		this->setWindowOpacity(0.0);
		_timer->stop();
		QWidget::setVisible(false);
	}*/
	QWidget::setVisible(visible);
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
void SearchDialog::processResults(Request type, const QStandardItemList &results)
{
	qDebug() << Q_FUNC_INFO << sender();

	QListView *listToProcess = NULL;
	switch (type) {
	case Artist:
		listToProcess = _artists;
		break;
	case Album:
		listToProcess = _albums;
		break;
	case Track:
		listToProcess = _tracks;
		break;
	}
	QStandardItemModel *m = qobject_cast<QStandardItemModel*>(listToProcess->model());
	for (int i = 0; i < results.size(); i++) {
		m->insertRow(0, results.at(i));
	}
	m->sort(0);
	//listToProcess->setMaximumHeight(listToProcess->count() * listToProcess->sizeHintForRow(0));
	//listToProcess->setMinimumHeight(listToProcess->count() * listToProcess->sizeHintForRow(0));
	//qDebug() << "number of items" << listToProcess->count();

	/*int h = qMax(_artists->count() * _artists->sizeHintForRow(0), iconArtists->height()) +
			qMax(_albums->count() * _albums->sizeHintForRow(0), iconAlbums->height()) +
			qMax(_tracks->count() * _tracks->sizeHintForRow(0), iconTracks->height());*/
	int h = 300;
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
	//this->close();
	this->setVisible(false);
	this->setGeometry(_oldRect);
}

void SearchDialog::appendSelectedItem(const QModelIndex &index)
{
	qDebug() << Q_FUNC_INFO << "not implemented";
	const QStandardItemModel *m = qobject_cast<const QStandardItemModel*>(index.model());
	qDebug() << m->itemFromIndex(index)->text();
}

/** Local search for matching expressions. */
void SearchDialog::search(const QString &text)
{
	qDebug() << Q_FUNC_INFO;
	if (!_checkBoxLibrary->isChecked()) {
		return;
	}

	if (!_db.isOpen()) {
		_db.open();
	}

	/// XXX: Factorize this, 3 times the (almost) same code
	QSqlQuery qSearchForArtists(_db);
	qSearchForArtists.prepare("SELECT DISTINCT artist FROM tracks WHERE artist like :t LIMIT 5");
	qSearchForArtists.bindValue(":t", "%" + text + "%");
	if (qSearchForArtists.exec()) {
		QList<QStandardItem*> artistList;
		while (qSearchForArtists.next()) {
			QStandardItem *artist = new QStandardItem(qSearchForArtists.record().value(0).toString());
			artist->setData(_checkBoxLibrary->text(), Qt::UserRole + 1);
			artistList.append(artist);
		}
		this->processResults(Artist, artistList);
	}

	QSqlQuery qSearchForAlbums(_db);
	qSearchForAlbums.prepare("SELECT DISTINCT album, COALESCE(artistAlbum, artist) FROM tracks WHERE album like :t LIMIT 5");
	qSearchForAlbums.bindValue(":t", "%" + text + "%");
	if (qSearchForAlbums.exec()) {
		QList<QStandardItem*> albumList;
		while (qSearchForAlbums.next()) {
			QStandardItem *album = new QStandardItem(qSearchForAlbums.record().value(0).toString() + " – " + qSearchForAlbums.record().value(1).toString());
			/// XXX create enum
			album->setData(_checkBoxLibrary->text(), Qt::UserRole + 1);
			albumList.append(album);
		}
		this->processResults(Album, albumList);
	}

	QSqlQuery qSearchForTracks(_db);
	qSearchForTracks.prepare("SELECT DISTINCT title, COALESCE(artistAlbum, artist) FROM tracks WHERE title like :t LIMIT 5");
	qSearchForTracks.bindValue(":t", "%" + text + "%");
	if (qSearchForTracks.exec()) {
		QList<QStandardItem*> trackList;
		while (qSearchForTracks.next()) {
			QStandardItem *track = new QStandardItem(qSearchForTracks.record().value(0).toString() + " – " + qSearchForTracks.record().value(1).toString());
			track->setData(_checkBoxLibrary->text(), Qt::UserRole + 1);
			trackList.append(track);
		}
		this->processResults(Track, trackList);
	}

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
