#include "coverfetcher.h"

#include <settings.h>
#include <model/sqldatabase.h>
#include <filehelper.h>
#include <cover.h>

#include "browseimagewidget.h"
#include "providers/amazonprovider.h"
#include "providers/musicbrainzprovider.h"
#include "fetchdialog.h"

#include <QCheckBox>
#include <QGroupBox>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QStackedLayout>
#include <QStackedWidget>

#include <QtDebug>

CoverFetcher::CoverFetcher(QObject *parent)
	: QObject(parent)
	, _manager(new QNetworkAccessManager(this))
{
	Settings *settings = Settings::instance();
	if (!settings->value("providers/coverValueSize").isValid()) {
		settings->setValue("providers/coverValueSize", 250);
		settings->setValue("providers/integrateCoverToFiles", true);
	}
	if (!settings->value("providers/musicbrainzCheckBox").isValid()) {
		settings->setValue("providers/musicbrainzCheckBox", true);
	}
	if (!settings->value("providers/amazonCheckBox").isValid()) {
		settings->setValue("providers/amazonCheckBox", true);
	}
	if (settings->value("providers/musicbrainzCheckBox").toBool()) {
		_providers.append(new MusicBrainzProvider(_manager));
	}
	if (settings->value("providers/amazonCheckBox").toBool()) {
		_providers.append(new AmazonProvider(_manager));
	}

	// Dispatch replies to specialized class
	connect(_manager, &QNetworkAccessManager::finished, this, [=](QNetworkReply *reply) {
		for (CoverArtProvider *cp : _providers) {
			if (cp->type() == reply->property("type").toInt()) {
				cp->dispatchReply(reply);
			}
		}
	});
}

CoverFetcher::~CoverFetcher()
{
	qDeleteAll(_providers);
}

/** Entry point. */
void CoverFetcher::fetch(SelectedTracksModel *selectedTracksModel)
{
	FetchDialog *fetchDialog = new FetchDialog(_providers);

	/// XXX: wow, kind of hack no?
	connect(fetchDialog, &FetchDialog::refreshView, this, [=]() {
		selectedTracksModel->updateSelectedTracks();
	});

	SqlDatabase db;
	db.open();

	// Format and concatenate all tracks in one big string. Replaces single quote with double quote
	/// FIXME
	QStringList tracks;
	for (QUrl u : selectedTracksModel->selectedTracks()) {
		tracks << u.toLocalFile();
	}
	QString l = tracks.join("\",\"").prepend("\"").append("\"");

	QString strArtistsAlbums = "SELECT artistAlbum, album, cover, internalCover " \
		"FROM cache WHERE uri IN (" + l + ") GROUP BY artistAlbum, album, cover ORDER BY artistAlbum, album";

	QSqlQuery qArtistsAlbums(db);
	qArtistsAlbums.exec(strArtistsAlbums);

	QString prevArtist = "";
	int size = Settings::instance()->value("providers/coverValueSize").toInt();
	QSize s(size, size);
	while (qArtistsAlbums.next()) {
		QString artistAlbum = qArtistsAlbums.record().value(0).toString();
		QString album = qArtistsAlbums.record().value(1).toString();
		QString cover = qArtistsAlbums.record().value(2).toString();
		QString internalCover = qArtistsAlbums.record().value(3).toString();

		// Send a new request for fetching artists only if it's a new one
		if (artistAlbum != prevArtist) {
			QLabel *labelArtist = new QLabel("Artist: " + artistAlbum);
			fetchDialog->scrollAreaWidgetContents->layout()->addWidget(labelArtist);
		}

		// Query all registered providers
		for (CoverArtProvider *cp : _providers) {
			QUrl url = cp->query(artistAlbum, album);
			QNetworkRequest request(url);
			request.setAttribute(QNetworkRequest::User, CoverArtProvider::FO_Search);
            request.setHeader(QNetworkRequest::UserAgentHeader, "MiamPlayer/0.9.0 ( https://github.com/MBach/Miam-Player )" );
			QNetworkReply *n = _manager->get(request);
			n->setProperty("type", cp->type());
			n->setProperty("requestType", CoverArtProvider::FO_Search);
			n->setProperty("artist", artistAlbum);
			n->setProperty("album", album);
		}

		QGroupBox *templateCover = new QGroupBox;
		templateCover->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
		templateCover->setTitle(tr("Album « %1 »").arg(album));
		templateCover->setProperty("album", album);
		QHBoxLayout *hbox = new QHBoxLayout;
		hbox->setMargin(0);
		templateCover->setLayout(hbox);

		QLabel *currentCover = new QLabel;
		currentCover->setScaledContents(true);

		QPixmap p;
		if (!internalCover.isEmpty()) {
			FileHelper fh(internalCover);
			Cover *c = fh.extractCover();
			if (c && p.loadFromData(c->byteArray())) {
				currentCover->setPixmap(p);
			}
			delete c;
		} else {
			if (cover.isEmpty()) {
				p.load(":/icons/disc");
			} else {
				p.load(cover);
			}
			currentCover->setPixmap(p);
		}
		currentCover->setMinimumSize(s);
		currentCover->setMaximumSize(s);

		QStackedWidget *remoteCovers = new QStackedWidget;
		remoteCovers->setMinimumSize(s);
		remoteCovers->setMaximumSize(s);
		remoteCovers->setProperty("artistAlbum", artistAlbum);
		remoteCovers->setProperty("album", album);

		BrowseImageWidget *biw = new BrowseImageWidget(remoteCovers);
		biw->setMinimumSize(s);
		biw->setMaximumSize(s);

		QStackedLayout *stackedLayout = new QStackedLayout;
		stackedLayout->setStackingMode(QStackedLayout::StackAll);
		stackedLayout->addWidget(remoteCovers);
		stackedLayout->addWidget(biw);
		stackedLayout->setMargin(0);
		stackedLayout->setSpacing(0);

		hbox->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));
		hbox->addWidget(currentCover);
		QWidget *w = new QWidget;
		w->setObjectName("container");
		w->setLayout(stackedLayout);
		w->setMinimumSize(s);
		w->setMaximumSize(s);
		hbox->addWidget(w);
		hbox->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));
		fetchDialog->scrollAreaWidgetContents->layout()->addWidget(templateCover);
		prevArtist = artistAlbum;
	}

	QSpacerItem *vSpacer = new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::Expanding);
	fetchDialog->scrollAreaWidgetContents->layout()->addItem(vSpacer);
	fetchDialog->show();
	fetchDialog->activateWindow();
}

/** When one is checking items in the list, providers are added or removed dynamically. */
void CoverFetcher::manageProvider(bool enabled, QCheckBox *checkBox)
{
	Settings::instance()->setValue("providers/" + checkBox->objectName(), enabled);
	if (enabled) {
		int type = checkBox->property("type").toInt();
		CoverArtProvider *cap = nullptr;
		switch(type) {
		case CoverArtProvider::PT_MusicBrainz:
			cap = new MusicBrainzProvider(_manager);
			break;
		case CoverArtProvider::PT_Amazon:
			cap = new AmazonProvider(_manager);
			break;
		/// TODO: other providers
		default:
			break;
		}
		if (cap != nullptr) {
			_providers.append(cap);
		}
	} else {
		QListIterator<CoverArtProvider*> it(_providers);
		while (it.hasNext()) {
			auto cap = it.next();
			if (cap->type() == checkBox->property("type").toInt()) {
				_providers.removeAll(cap);
				delete cap;
				break;
			}
		}
	}
}
