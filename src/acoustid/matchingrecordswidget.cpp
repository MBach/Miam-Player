#include "matchingrecordswidget.h"

#include <QtDebug>

MatchingRecordsWidget::MatchingRecordsWidget(QWidget *parent)
	: QWidget(parent)
{
	setupUi(this);
	//recordingsListWidget->setItemDelegate(new CoverWidgetItemDelegate(recordingsListWidget));

	connect(removeReleasesButton, &QToolButton::clicked, this, [=]() {
		_releases.clear();
		emit aboutToHide();
	});
	connect(recordingsListWidget, &QListWidget::currentRowChanged, this, [=](int row) {
		emit releaseChanged(_releases.at(row));
	});
}

void MatchingRecordsWidget::addRelease(const MusicBrainz::Release &release)
{
	qDebug() << Q_FUNC_INFO << "work in progress";
	if (_releases.contains(release)) {
		int index = _releases.indexOf(release);
		MusicBrainz::Release previous = _releases.at(index);
		previous.tracks.unite(release.tracks);
		_releases.replace(index, previous);
		qDebug() << Q_FUNC_INFO << "release" << release.title << "was already added, skipping or merging";
		return;
	} else {
		_releases.insert(recordingsListWidget->count(), release);
	}
	QIcon defaultIcon(":/icons/disc");
	QString defaultText;
	defaultText = release.title + " / " + QString::number(release.trackCount) + " / " + QString::number(release.year)
			+ " / " + release.country + " / " + release.format;
	QListWidgetItem *releaseItem = new QListWidgetItem(defaultIcon, defaultText, recordingsListWidget);
	releaseItem->setFlags(releaseItem->flags() | Qt::ItemIsUserCheckable);
	releaseItem->setCheckState(Qt::Unchecked);
	releaseItem->setData(Qt::UserRole, QVariant::fromValue(release));

	if (recordingsListWidget->count() == 1) {
		recordingsListWidget->setCurrentRow(0);
		recordingsListWidget->setFocus();
	}
}

void MatchingRecordsWidget::autoSelectFirstResult()
{
	qDebug() << Q_FUNC_INFO << "tracks were analyzed";
	QListWidgetItem *first = recordingsListWidget->item(0);
	if (first) {
		first->setSelected(true);
		first->setCheckState(Qt::Checked);
		emit releaseChanged(_releases.at(0));
	}
}
