#include "acoustid.h"
#include "settings.h"

#include <QPushButton>

#include <QtDebug>

QString AcoustId::_apiKey = "iocykM04";
QString AcoustId::_wsAcoustID = "http://api.acoustid.org/v2/lookup";

AcoustId::AcoustId(QObject *parent)
	: QObject(parent)
	, _requestPool(new RequestPool(this))
	, _chromaprint(new QChromaprint(this))
	, _matchingRecordsWidget(new MatchingRecordsWidget)
	, _analyzeButton(nullptr)
	//, _tableWidget(nullptr)
{
	connect(_requestPool, &RequestPool::releaseFound, _matchingRecordsWidget, &MatchingRecordsWidget::addRelease);
	connect(this, &AcoustId::tracksAnalyzed, _matchingRecordsWidget, &MatchingRecordsWidget::autoSelectFirstResult);

	connect(_matchingRecordsWidget, &MatchingRecordsWidget::releaseChanged, this, [=](const MusicBrainz::Release &release) {
		qDebug() << Q_FUNC_INFO << "load release info for" << release.title << "and then update table";
		qDebug() << Q_FUNC_INFO << "what are the tracks in tag editor?";
		/*QModelIndexList list = _tableWidget->selectionModel()->selectedRows();
		qDebug() << Q_FUNC_INFO << list;
		for (int i = 0; i < list.count(); i++) {
			QModelIndex index = list.at(i);
			if (index.column() == Miam::COL_Filename) {
				int r = index.row();
				QTableWidgetItem *filename = _tableWidget->item(r, Miam::COL_Filename);

				/// XXX: hidden data!
				MusicBrainz::Track t = release.track(filename->data(Qt::UserRole).toString());

				QTableWidgetItem *title = _tableWidget->item(r, Miam::COL_Title);
				QTableWidgetItem *trackNumber = _tableWidget->item(r, Miam::COL_Track);
				QTableWidgetItem *artist = _tableWidget->item(r, Miam::COL_Artist);
				QTableWidgetItem *artistAlbum = _tableWidget->item(r, Miam::COL_ArtistAlbum);
				QTableWidgetItem *album = _tableWidget->item(r, Miam::COL_Album);
				QTableWidgetItem *year = _tableWidget->item(r, Miam::COL_Year);
				//QTableWidgetItem *genre = _tableWidget->item(r, Miam::COL_Genre);
				QTableWidgetItem *disc = _tableWidget->item(r, Miam::COL_Disc);

				title->setText(t.title);
				trackNumber->setText(QString::number(t.position));
				artist->setText(release.artist.name);
				if (!t.artist->name.isEmpty()) {
					artistAlbum->setText(t.artist->name);
				}
				album->setText(release.title);
				year->setText(QString::number(release.year));
				disc->setText(QString::number(release.disc));
			}
		}*/
	});
}

AcoustId::~AcoustId()
{}

/*void AcoustId::setExtensibleLayout(QHBoxLayout *layout)
{
	_analyzeButton = new QPushButton(QIcon(":/acoustid/magic"), tr("Analyze"));
	_analyzeButton->setIconSize(QSize(24, 24));
	_analyzeButton->setEnabled(false);
	connect(_analyzeButton, &QPushButton::clicked, this, &AcoustId::start);
	layout->addWidget(_analyzeButton);
}*/

/*void AcoustId::setStackWidget(QStackedWidget *sw)
{
	_stackedWidget = sw;

	sw->addWidget(_matchingRecordsWidget);
	connect(_matchingRecordsWidget, &MatchingRecordsWidget::aboutToHide, this, [=]() {
		_matchingRecordsWidget->recordingsListWidget->clear();
		if (sw->count() == 1) {
			sw->hide();
		}
	});
}*/

/*void AcoustId::setTagEditorWidget(QTableWidget *tableWidget)
{
	_tableWidget = tableWidget;
	connect(tableWidget, &QTableWidget::itemSelectionChanged, this, [=]() {
		_analyzeButton->setDisabled(tableWidget->selectionModel()->selectedIndexes().isEmpty());
	});
}*/

void AcoustId::start()
{
	/*if (!_selectedTracksModel->selectedTracks().isEmpty()) {
		_stackedWidget->setVisible(true);

		QString appName = QCoreApplication::instance()->applicationName();
		QString appVersion = QCoreApplication::instance()->applicationVersion();
		QString client = appName.append(appVersion);

		for (int i = 0; i < _selectedTracksModel->selectedTracks().count(); i++) {
			QString track = _selectedTracksModel->selectedTracks().at(i).toLocalFile();
			if (_chromaprint->start(track)) {

				QString fingerprint = _chromaprint->fingerprint();
				if (!fingerprint.isEmpty()) {

					QUrlQuery urlQuery;
					urlQuery.addQueryItem("format", "json");
					urlQuery.addQueryItem("client", AcoustId::_apiKey);
					urlQuery.addQueryItem("duration", QString::number(_chromaprint->duration()));
					urlQuery.addQueryItem("meta", "recordings+releasegroups+releases+tracks");
					//qDebug() << fingerprint;
					urlQuery.addQueryItem("fingerprint", fingerprint);

					QNetworkRequest request(QUrl::fromEncoded(_wsAcoustID.toLatin1()));
					request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
					//request.setRawHeader("Content-Encoding", "gzip");
					request.setRawHeader("User-Agent", client.toLatin1());

					_requestPool->add(track, request, urlQuery, _chromaprint->duration());
				}
			}
		}
	}*/
}
