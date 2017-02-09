#include "tableview.h"

#include <model/sqldatabase.h>
#include <libraryfilterproxymodel.h>
#include <libraryscrollbar.h>
#include <settingsprivate.h>

#include <QGuiApplication>
#include <QHeaderView>
#include <QPainter>
#include <QRegularExpression>
#include <QScrollBar>
#include <QSqlQuery>
#include <QSqlRecord>

#include <QLabel>
#include <QVBoxLayout>

#include <QtDebug>

TableView::TableView(QWidget *parent)
	: QTableView(parent)
	, _model(new UniqueLibraryItemModel(this))
	, _jumpToWidget(new JumpToWidget(this))
	, _skipCount(1)
	, _actionSendToTagEditor(new QAction(this))
	, _artistHeader(new QWidget(this))
{
	_model->proxy()->setDynamicSortFilter(false);
	this->setModel(_model->proxy());
	this->setVerticalScrollMode(ScrollPerPixel);
	LibraryScrollBar *vScrollBar = new LibraryScrollBar(this);
	this->setVerticalScrollBar(vScrollBar);

	QVBoxLayout *vBoxLayout = new QVBoxLayout;
	vBoxLayout->setSpacing(0);
	vBoxLayout->setContentsMargins(0, 0, 0, 0);
	QLabel *artist = new QLabel(_artistHeader);
	artist->setContentsMargins(0, 0, 0, 0);
	artist->setText("");
	vBoxLayout->addWidget(artist);
	_artistHeader->setContentsMargins(0, 0, 0, 0);
	_artistHeader->setLayout(vBoxLayout);

	connect(_jumpToWidget, &JumpToWidget::aboutToScrollTo, this, &TableView::jumpTo);
	connect(_model->proxy(), &UniqueLibraryFilterProxyModel::aboutToHighlightLetters, _jumpToWidget, &JumpToWidget::highlightLetters);
	connect(vScrollBar, &QAbstractSlider::valueChanged, this, [=](int) {
		QModelIndex iTop = indexAt(viewport()->rect().topRight());
		QModelIndex sourceTop = _model->proxy()->mapToSource(iTop);
		QStandardItem *item = _model->itemFromIndex(_model->index(sourceTop.row(), sourceTop.column()));
		if (item) {
			if (item->type() == Miam::IT_Artist) {
				artist->setText(item->text());
			} else {
				artist->setText(item->data(Miam::DF_Artist).toString());
			}
		}
		_jumpToWidget->setCurrentLetter(_model->currentLetter(iTop));
	});
	horizontalHeader()->resizeSection(0, Settings::instance()->coverSizeUniqueLibrary());

	connect(selectionModel(), &QItemSelectionModel::selectionChanged, [=](const QItemSelection &, const QItemSelection &) {
		if (viewport()) {
			setDirtyRegion(QRegion(viewport()->rect()));
		}
	});

	connect(_actionSendToTagEditor, &QAction::triggered, this, [=]() {
		int c = this->selectedIndexes().count();
		if (Miam::showWarning(tr("tag editor"), c) == QMessageBox::Ok) {
			emit sendToTagEditor(this->selectedTracks());
		}
	});
	_menu.addAction(_actionSendToTagEditor);

	// Theme
	SettingsPrivate *settingsPrivate = SettingsPrivate::instance();
	QFont f = settingsPrivate->font(SettingsPrivate::FF_Library);
	auto loadFont = [this] (const QFont &f) -> void {
		this->setFont(f);
		QFontMetrics fm = this->fontMetrics();
		verticalHeader()->setDefaultSectionSize(fm.height());
	};
	loadFont(f);

	connect(settingsPrivate, &SettingsPrivate::fontHasChanged, this, [=](SettingsPrivate::FontFamily ff, const QFont &newFont) {
		if (ff == SettingsPrivate::FF_Library) {
			loadFont(newFont);
		}
	});
	this->adjust();
	this->installEventFilter(this);
}

bool TableView::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::ShowToParent) {
		qDebug() << Q_FUNC_INFO << "so what?";
		//QShowEvent *showEvent = static_cast<QShowEvent*>(event);
		_artistHeader->setMinimumWidth(this->width());
	} else if (event->type() == QEvent::ShortcutOverride) {
		QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
		if (keyEvent->modifiers().testFlag(Qt::NoModifier)) {
			// If one has assigned a simple key like 'N' to 'Skip Forward' we don't actually want to skip the track
			if (65 <= keyEvent->key() && keyEvent->key() <= 90) {
				// We don't want this event to be propagated
				keyEvent->accept();
				return true;
			}
		} else {
			keyEvent->ignore();
			return false;
		}
	}
	return QTableView::eventFilter(obj, event);
}

TableView::~TableView()
{
	if (selectionModel()) {
		selectionModel()->disconnect();
	}
}

/** Adjust row height of last track when tracks in an album have an height lower than cover size. */
void TableView::adjust()
{
	int rowHeightForAlbum = 0;
	int dss = verticalHeader()->defaultSectionSize();
	int coverSize = Settings::instance()->coverSizeUniqueLibrary();
	QStandardItem *previous = nullptr;
	for (int i = 0; i < model()->proxy()->rowCount(); i++) {

		QModelIndex index = model()->proxy()->index(i, 1);
		auto item = model()->itemFromIndex(model()->proxy()->mapToSource(index));
		if (item && item->type() == Miam::IT_Track) {
			rowHeightForAlbum += dss;
			previous = item;
		} else {
			// Set new row height for previous track
			if (rowHeightForAlbum != 0 && rowHeightForAlbum < coverSize && previous != nullptr) {
				setRowHeight(index.row() - 1, coverSize - rowHeightForAlbum + dss / 2);
			}
			rowHeightForAlbum = 0;
			previous = nullptr;
		}
	}
}

/** Redefined to disable search in the table and trigger jumpToWidget's action. */
void TableView::keyboardSearch(const QString &search)
{
	// If one has assigned a simple key like 'N' to 'Skip Forward' we don't actually want to skip the track
	// IMHO, it's better to trigger the JumpTo widget to 'N' section
	static QRegularExpression az("[a-z]", QRegularExpression::CaseInsensitiveOption | QRegularExpression::OptimizeOnFirstUsageOption);
	if (az.match(search).hasMatch()) {
		if (jumpToWidget()->currentLetter() == search.toUpper().at(0)) {
			_skipCount++;
		} else {
			_skipCount = 1;
		}
		this->jumpTo(search);
	}
}

QList<QUrl> TableView::selectedTracks()
{
	QStringList results;
	auto proxy = model()->proxy();
	QStringList artists, albums, tracks;
	for (QModelIndex i : selectedIndexes()) {
		if (QStandardItem *item = model()->itemFromIndex(proxy->mapToSource(i))) {
			if (item->type() == Miam::IT_Artist) {
				artists << '"' + item->data(Miam::DF_NormalizedString).toString() + '"';
			} else if (item->type() == Miam::IT_Album) {
				albums << '"' + item->data(Miam::DF_NormAlbum).toString() + '"';
			} else if (item->type() == Miam::IT_Track) {
				tracks << '"' + item->data(Miam::DF_URI).toString() + '"';
			}
		}
	}
	SqlDatabase db;
	QSqlQuery q(db);
	if (q.exec("SELECT uri FROM cache WHERE artistNormalized IN (" + artists.join(",") + ")")) {
		while (q.next()) {
			results << q.record().value(0).toString();
		}
	}
	if (q.exec("SELECT uri FROM cache WHERE albumNormalized IN (" + albums.join(",") + ")")) {
		while (q.next()) {
			results << q.record().value(0).toString();
		}
	}
	if (q.exec("SELECT uri FROM cache WHERE uri IN (" + tracks.join(",") + ")")) {
		while (q.next()) {
			results << q.record().value(0).toString();
		}
	}

	results.removeDuplicates();
	results.sort();
	QList<QUrl> urls;
	for (QString result : results) {
		urls << QUrl::fromLocalFile(result);
	}
	return urls;
}

void TableView::updateSelectedTracks()
{

}

void TableView::contextMenuEvent(QContextMenuEvent *e)
{
	if (selectedIndexes().count() == 1) {
		QStandardItem *item = model()->itemFromIndex(model()->proxy()->mapToSource(selectedIndexes().first()));
		if (item) {
			_actionSendToTagEditor->setText(tr("Send '%1' to the tag editor").arg(item->text().replace("&", "&&")));
		}
	} else if (selectedIndexes().count() > 1) {
		_actionSendToTagEditor->setText(tr("Send to tag editor"));
	}
	_menu.exec(e->globalPos());
}

/** Redefined to keep displayed covers untouched. */
void TableView::mouseMoveEvent(QMouseEvent *event)
{
	// Don't allow click on first column which contains the cover (it can erase parts of painted image)
	if (columnAt(event->pos().x()) != 0) {
		QTableView::mouseMoveEvent(event);
	}
}

/** Redefined to keep displayed covers untouched. */
void TableView::mousePressEvent(QMouseEvent *event)
{
	// Don't allow click on first column which contains the cover (it can erase parts of painted image)
	if (columnAt(event->pos().x()) != 0) {
		QTableView::mousePressEvent(event);
	}
}

void TableView::paintEvent(QPaintEvent *event)
{
	int wVerticalScrollBar = 0;
	if (verticalScrollBar()->isVisible()) {
		wVerticalScrollBar = verticalScrollBar()->width();
	}
	if (QGuiApplication::isLeftToRight()) {
		///XXX: magic number
		_jumpToWidget->move(frameGeometry().right() - 22 - wVerticalScrollBar, 0);
	} else {
		_jumpToWidget->move(frameGeometry().left() + wVerticalScrollBar, 0);
	}

	if (_model->rowCount() == 0) {
		QPainter p(this->viewport());
		p.drawText(this->viewport()->rect(), Qt::AlignCenter, tr("No matching results were found"));
	} else {
		QTableView::paintEvent(event);
	}
}

void TableView::jumpTo(const QString &letter)
{
	SqlDatabase db;
	QSqlQuery firstArtist(db);
	firstArtist.prepare("SELECT artist FROM cache WHERE artist LIKE ? ORDER BY artist COLLATE NOCASE LIMIT ?");
	firstArtist.addBindValue(letter + "%");
	firstArtist.addBindValue(_skipCount);
	if (firstArtist.exec() && firstArtist.last()) {
		if (_skipCount != firstArtist.at() + 1) {
			firstArtist.first();
			_skipCount = 1;
		}
		for (QStandardItem *i : _model->findItems(firstArtist.record().value(0).toString(), Qt::MatchExactly, 1)) {
			if (i->type() == Miam::IT_Artist) {
				this->scrollTo(_model->proxy()->mapFromSource(i->index()), PositionAtTop);
				break;
			}
		}
	}
}
