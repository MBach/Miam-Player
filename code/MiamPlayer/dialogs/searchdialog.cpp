#include "searchdialog.h"

#include <QTimer>
#include <QStandardItemModel>
#include <QStylePainter>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>

#include "mainwindow.h"
#include "settings.h"
#include "pluginmanager.h"

#include <QtDebug>

/** Constructor. */
SearchDialog::SearchDialog(const SqlDatabase &db, MainWindow *mainWindow) :
	AbstractSearchDialog(mainWindow, Qt::Widget), _mainWindow(mainWindow), _db(db), _isMaximized(false)
{
	this->setupUi(this);
	_artists->setModel(new QStandardItemModel(this));
	_albums->setModel(new QStandardItemModel(this));
	_tracks->setModel(new QStandardItemModel(this));

	// Init map with empty values
	foreach (QListView *list, this->findChildren<QListView*>()) {
		_hiddenItems.insert(list, QList<QStandardItem*>());
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
	connect(labelSearchMore, &QLabel::linkActivated, this, &SearchDialog::searchLabelWasClicked);

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
	connect(SettingsPrivate::getInstance(), &SettingsPrivate::fontHasChanged, this, [=](SettingsPrivate::FontFamily ff, const QFont &newFont) {
		if (ff == SettingsPrivate::FF_Library) {
			foreach (QWidget *o, this->findChildren<QWidget*>()) {
				o->setFont(newFont);
			}
		}
	});

	_mainWindow->installEventFilter(this);

	this->setVisible(false);
	_oldRect = this->geometry();

	connect(_artists, &QListView::doubleClicked, this, &SearchDialog::artistWasDoubleClicked);
	connect(_albums, &QListView::doubleClicked, this, &SearchDialog::albumWasDoubleClicked);
	connect(_tracks, &QListView::doubleClicked, this, &SearchDialog::trackWasDoubleClicked);
}

/** Required interface from AbstractSearchDialog class. */
void SearchDialog::addSource(QCheckBox *checkBox)
{
	int i = sources_layout->count(); // Default are: HSpacer
	checkBox->setFont(SettingsPrivate::getInstance()->font(SettingsPrivate::FF_Library));
	sources_layout->insertWidget(i - 1, checkBox);

	connect(checkBox, &QCheckBox::toggled, this, &SearchDialog::toggleItems);
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

bool SearchDialog::eventFilter(QObject *obj, QEvent *event)
{
	if (obj == _mainWindow && event->type() == QEvent::Resize) {
		if (this->isVisible() && _isMaximized) {
			this->move(0, 0);
			this->resize(_mainWindow->rect().size());
		}
	}
	return AbstractSearchDialog::eventFilter(obj, event);
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
}

void SearchDialog::aboutToProcessRemoteTracks(const std::list<RemoteTrack> &tracks)
{
	Playlist *p = _mainWindow->tabPlaylists->currentPlayList();
	p->insertMedias(-1, QList<RemoteTrack>::fromStdList(tracks));
	this->clear();
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
	_isMaximized = false;
	iconSearchMore->setPixmap(QPixmap(":/icons/search"));
	labelSearchMore->setText(tr("<a href='#more' style='text-decoration: none; color:#3399FF;'>Search for more results...</a>"));
	this->setVisible(false);
	this->setGeometry(_oldRect);
	_artists->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	_albums->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	_tracks->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void SearchDialog::artistWasDoubleClicked(const QModelIndex &artist)
{
	qDebug() << Q_FUNC_INFO << "not implemented";
}

void SearchDialog::albumWasDoubleClicked(const QModelIndex &album)
{
	qDebug() << Q_FUNC_INFO << "not implemented";
}

void SearchDialog::trackWasDoubleClicked(const QModelIndex &track)
{
	Playlist *p = _mainWindow->tabPlaylists->currentPlayList();
	QStringList l = QStringList() << track.data(DT_Identifier).toString();
	p->insertMedias(-1, l);
	this->clear();
}

void SearchDialog::appendSelectedItem(const QModelIndex &index)
{
	qDebug() << Q_FUNC_INFO;
	const QStandardItemModel *m = qobject_cast<const QStandardItemModel*>(index.model());

	// At this point, we have to decide if the object that has been double clicked is local or remote
	QStandardItem *item = m->itemFromIndex(index);
	qDebug() << item->text();

	QListView *list = qobject_cast<QListView*>(sender());

	Playlist *p = _mainWindow->tabPlaylists->currentPlayList();
	if (item->data(AbstractSearchDialog::DT_Origin).toString() == _checkBoxLibrary->text()) {
		QList<QMediaContent> tracks;
		// Local items: easy to process! (SQL request)
		if (list == _artists) {
			// Select all tracks from this Artist
		} else if (list == _albums) {
			// Select all tracks from this Album
		} else /*if (list == _tracks)*/ {
			// Nothing special
		}
		p->insertMedias(-1, tracks);
	} else {
		// Remote items: apply strategy pattern to get remote information depending on the caller
		QList<RemoteTrack> tracks;
		p->insertMedias(-1, tracks);
	}
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
			artist->setData(_checkBoxLibrary->text(), DT_Origin);
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
			album->setData(_checkBoxLibrary->text(), DT_Origin);
			albumList.append(album);
		}
		this->processResults(Album, albumList);
	}

	QSqlQuery qSearchForTracks(_db);
	qSearchForTracks.prepare("SELECT DISTINCT title, COALESCE(artistAlbum, artist), absPath FROM tracks WHERE title like :t LIMIT 5");
	qSearchForTracks.bindValue(":t", "%" + text + "%");
	if (qSearchForTracks.exec()) {
		QList<QStandardItem*> trackList;
		while (qSearchForTracks.next()) {
			QSqlRecord r = qSearchForTracks.record();
			QStandardItem *track = new QStandardItem(r.value(0).toString() + " – " + r.value(1).toString());
			track->setData(_checkBoxLibrary->text(), DT_Origin);
			track->setData(r.value(2), DT_Identifier);
			trackList.append(track);
		}
		this->processResults(Track, trackList);
	}

	_db.close();
}

/** Expand this dialog to all available space. */
void SearchDialog::searchLabelWasClicked(const QString &link)
{
	if (link == "#more") {
		_isMaximized = true;
		iconSearchMore->setPixmap(QPixmap(":/icons/back"));
		labelSearchMore->setText(tr("<a href='#less' style='text-decoration: none; color:#3399FF;'>Show less results</a>"));
		this->move(0, 0);
		this->resize(_mainWindow->rect().size());
		this->searchMoreResults();
		_artists->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
		_albums->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
		_tracks->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	} else {
		_isMaximized = false;
		iconSearchMore->setPixmap(QPixmap(":/icons/search"));
		labelSearchMore->setText(tr("<a href='#more' style='text-decoration: none; color:#3399FF;'>Search for more results...</a>"));
		_mainWindow->moveSearchDialog();
		this->resize(_oldRect.size());
		_artists->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		_albums->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		_tracks->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	}
}

/** Start search again more but fetch more results. */
void SearchDialog::searchMoreResults()
{
	/// TODO
	qDebug() << Q_FUNC_INFO << "not implemented";
}

void SearchDialog::toggleItems(bool enabled)
{
	QCheckBox *checkBox = qobject_cast<QCheckBox*>(sender());
	foreach (QListView *list, this->findChildren<QListView*>()) {
		QStandardItemModel *m = qobject_cast<QStandardItemModel*>(list->model());
		// Hiding / restoring items has to be done in 2-steps
		// First step is for finding items that are about to be moved
		// Second step is for iterating backward on marked items -> you cannot remove items on a single for loop
		if (enabled) {
			// Restore hidden items for every list
			QList<QStandardItem*> items = _hiddenItems.value(list);
			QList<int> indexes;
			for (int i = 0; i < items.size(); i++) {
				QStandardItem *item = items.at(i);
				// Extract only matching items
				if (item->data(AbstractSearchDialog::DT_Origin) == checkBox->text()) {
					indexes.prepend(i);
				}
			}

			// Moving back from hidden to visible
			for (int i = 0; i < indexes.size(); i++) {
				QStandardItem *item = items.takeAt(indexes.at(i));
				m->appendRow(item);
			}

			// Replace existing values with potentially empty list
			_hiddenItems.insert(list, items);
			m->sort(0);
		} else {
			// Hide items for every list
			QStandardItemModel *m = qobject_cast<QStandardItemModel*>(list->model());
			QList<QStandardItem*> items;
			QList<QPersistentModelIndex> indexes;
			for (int i = 0; i < m->rowCount(); i++) {
				QStandardItem *item = m->item(i, 0);
				if (item->data(AbstractSearchDialog::DT_Origin).toString() == checkBox->text()) {
					indexes << m->index(i, 0);
					// Default copy-constructor is protected!
					QStandardItem *copy = new QStandardItem(item->text());
					copy->setData(checkBox->text(), AbstractSearchDialog::DT_Origin);
					copy->setIcon(item->icon());
					items.append(copy);
				}
			}

			foreach (const QPersistentModelIndex &i, indexes) {
				m->removeRow(i.row());
			}

			// Finally, hide selected items
			if (!items.isEmpty()) {
				QList<QStandardItem*> hItems = _hiddenItems.value(list);
				hItems.append(items);
				_hiddenItems.insert(list, hItems);
			}
		}
	}
}
