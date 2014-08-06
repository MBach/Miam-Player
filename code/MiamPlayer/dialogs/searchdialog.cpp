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

SearchDialog::SearchDialog(const SqlDatabase &db, QWidget *parent) :
	QDialog(parent, Qt::Tool | Qt::FramelessWindowHint), _db(db)
{
	this->setupUi(this);

	QFont f = Settings::getInstance()->font(Settings::FF_Library);
	artists->setFont(f);
	albums->setFont(f);
	tracks->setFont(f);
	labelSearchMore->setFont(f);
	labelAggregated->setFont(f);
	checkBoxLibrary->setFont(f);

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

	foreach (QListWidget *list, findChildren<QListWidget*>()) {
		connect(list, &QListWidget::itemClicked, this, [=]() {
			foreach (QListWidget *otherList, findChildren<QListWidget*>()) {
				if (list != otherList) {
					otherList->clearSelection();
				}
			}
		});
	}

	QObjectList objectsToExtend;
	objectsToExtend << artists << albums << tracks << aggregated << this;
	PluginManager::getInstance()->registerExtensionPoint(this->metaObject()->className(), objectsToExtend);
}

SearchDialog::~SearchDialog()
{}

void SearchDialog::search(const QString &text)
{
	artists->clear();
	albums->clear();
	tracks->clear();

	if (!_db.isOpen()) {
		_db.open();
	}

	QSqlQuery qSearchForArtists(_db);
	qSearchForArtists.prepare("SELECT DISTINCT artist FROM tracks WHERE artist like :t LIMIT 5");
	qSearchForArtists.bindValue(":t", "%" + text + "%");
	if (qSearchForArtists.exec()) {
		while (qSearchForArtists.next()) {
			//qDebug() << qSearchForArtists.record().value(0);
			artists->addItem(qSearchForArtists.record().value(0).toString());
		}
	}
	artists->setMinimumHeight(artists->count() * artists->sizeHintForRow(0));

	QSqlQuery qSearchForAlbums(_db);
	qSearchForAlbums.prepare("SELECT DISTINCT album, COALESCE(artistAlbum, artist) FROM tracks WHERE album like :t LIMIT 5");
	qSearchForAlbums.bindValue(":t", "%" + text + "%");
	if (qSearchForAlbums.exec()) {
		while (qSearchForAlbums.next()) {
			//qDebug() << qSearchForAlbums.record().value(0);
			albums->addItem(qSearchForAlbums.record().value(0).toString() + " – " + qSearchForAlbums.record().value(1).toString());
		}
	}
	albums->setMinimumHeight(albums->count() * albums->sizeHintForRow(0));

	QSqlQuery qSearchForTracks(_db);
	qSearchForTracks.prepare("SELECT DISTINCT title, COALESCE(artistAlbum, artist) FROM tracks WHERE title like :t LIMIT 5");
	qSearchForTracks.bindValue(":t", "%" + text + "%");
	if (qSearchForTracks.exec()) {
		while (qSearchForTracks.next()) {
			//qDebug() << qSearchForTracks.record().value(0);
			tracks->addItem(qSearchForTracks.record().value(0).toString() + " – " + qSearchForTracks.record().value(1).toString());
		}
	}
	tracks->setMinimumHeight(tracks->count() * tracks->sizeHintForRow(0));

	int items = artists->count() + albums->count() + tracks->count();
	items = items * artists->sizeHintForRow(0);
	int minW = qMax(labelArtists->width() + artists->sizeHintForColumn(0), 400);
	this->resize(minW, items + labelSearchMore->height());

	_db.close();
}

void SearchDialog::setVisible(bool visible)
{
	if (visible) {
		QDialog::setVisible(true);
		if (!_timer->isActive()) {
			this->animate(0.0, 1.0);
		}
		_timer->start();
	} else {
		QDialog::setVisible(false);
		this->setWindowOpacity(0.0);
		_timer->stop();
	}
}

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

void SearchDialog::animate(qreal startValue, qreal stopValue)
{
	_animation->setStartValue(startValue);
	_animation->setEndValue(stopValue);
	_animation->start();
}

void SearchDialog::clear()
{
	this->close();
}

void SearchDialog::searchMoreResults(const QString &link)
{
	if (link == "#") {
		QWidget *p = this;
		QRect mainWindowRect;
		QPoint origin;
		while (p->parentWidget() != NULL) {
			p = p->parentWidget();
			if (p->parentWidget() == NULL) {
				mainWindowRect = p->rect();
				origin = p->mapToGlobal(mainWindowRect.topLeft());
			}
		}
		this->move(origin);
		this->resize(mainWindowRect.size());
	}
}
