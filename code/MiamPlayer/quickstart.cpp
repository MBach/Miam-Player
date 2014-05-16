#include "quickstart.h"

#include "columnutils.h"
#include "filehelper.h"
#include "nofocusitemdelegate.h"

#include <QStandardPaths>
#include <QThread>

#include <QtDebug>

const QList<int> QuickStart::ratios = QList<int>() << 0 << 3 << 2;

QuickStart::QuickStart(QWidget *parent) :
	QWidget(parent), _totalMusicFiles(0), _worker(NULL), _qsse(NULL)
{
	setupUi(this);

	quickStartTableWidget->setItemDelegate(new NoFocusItemDelegate(this));

	connect(quickStartTableWidget, &QTableWidget::itemClicked, this, &QuickStart::checkRow);
	this->installEventFilter(this);
}

bool QuickStart::eventFilter(QObject *, QEvent *e)
{
	if (e->type() == QEvent::Show || e->type() == QEvent::Resize) {
		ColumnUtils::resizeColumns(quickStartTableWidget, ratios);
		return true;
	} else {
		return false;
	}
}

/** The first time the player is launched, this function will scan for multimedia files. */
void QuickStart::searchMultimediaFiles()
{
	while (quickStartTableWidget->rowCount() > 0) {
		quickStartTableWidget->removeRow(0);
	}
	if (QStandardPaths::standardLocations(QStandardPaths::MusicLocation).first().isEmpty()) {
		quickStartGroupBox->hide();
		otherwiseLabel->hide();
	} else {
		QThread *worker = new QThread(this);
		_qsse = new QuickStartSearchEngine();
		_qsse->moveToThread(worker);
		connect(_qsse, SIGNAL(folderScanned(QFileInfo,int)), this, SLOT(insertRow(QFileInfo,int)));
		connect(worker, &QThread::started, _qsse, &QuickStartSearchEngine::doSearch);
		connect(worker, &QThread::finished, this, &QuickStart::insertFirstRow);
		worker->start();
	}
}

void QuickStart::checkRow(QTableWidgetItem *i)
{
	int row = i->row();
	bool invertBehaviour = i->column() == 0;
	// First row is the master checkbox
	Qt::CheckState state = quickStartTableWidget->item(row, 0)->checkState();
	if (row == 0) {
		if (invertBehaviour) {
			(state == Qt::Checked || state == Qt::PartiallyChecked) ? state = Qt::Checked : state = Qt::Unchecked;
		} else {
			(state == Qt::Checked || state == Qt::PartiallyChecked) ? state = Qt::Unchecked : state = Qt::Checked;
		}
		for (int r = 1; r < quickStartTableWidget->rowCount(); r++) {
			quickStartTableWidget->item(r, 0)->setCheckState(state);
		}
	} else if (!invertBehaviour) {
		if (state == Qt::Checked) {
			quickStartTableWidget->item(row, 0)->setCheckState(Qt::Unchecked);
		} else {
			quickStartTableWidget->item(row, 0)->setCheckState(Qt::Checked);
		}
	}

	// Each time one clicks on a row, it's necessary to check if the apply button has to be enabled
	bool atLeastOneFolderIsSelected = false;
	bool allFoldersAreSelected = true;
	for (int r = 1; r < quickStartTableWidget->rowCount(); r++) {
		atLeastOneFolderIsSelected = atLeastOneFolderIsSelected || quickStartTableWidget->item(r, 0)->checkState() == Qt::Checked;
		allFoldersAreSelected = allFoldersAreSelected && quickStartTableWidget->item(r, 0)->checkState() == Qt::Checked;
	}
	quickStartApplyButton->setEnabled(atLeastOneFolderIsSelected);
	if (allFoldersAreSelected) {
		quickStartTableWidget->item(0, 0)->setCheckState(Qt::Checked);
	} else if (atLeastOneFolderIsSelected) {
		quickStartTableWidget->item(0, 0)->setCheckState(Qt::PartiallyChecked);
	} else {
		quickStartTableWidget->item(0, 0)->setCheckState(Qt::Unchecked);
	}
}

/** Insert above other rows a new one with a Master checkbox to select/unselect all. */
void QuickStart::insertFirstRow()
{
	// But only if some music was found on default music folder
	if (_totalMusicFiles == 0) {
		quickStartGroupBox->hide();
		otherwiseLabel->hide();
	} else {
		ColumnUtils::resizeColumns(quickStartTableWidget, ratios);

		QTableWidgetItem *masterCheckBox = new QTableWidgetItem;
		masterCheckBox->setFlags(masterCheckBox->flags() | Qt::ItemIsTristate | Qt::ItemIsUserCheckable);

		bool atLeastOneFolderIsEmpty = false;
		for (int r = 0; r < quickStartTableWidget->rowCount(); r++) {
			atLeastOneFolderIsEmpty = atLeastOneFolderIsEmpty || quickStartTableWidget->item(r, 0)->checkState() == Qt::Unchecked;
		}
		if (atLeastOneFolderIsEmpty) {
			masterCheckBox->setCheckState(Qt::PartiallyChecked);
		} else {
			masterCheckBox->setCheckState(Qt::Checked);
		}

		QTableWidgetItem *totalFiles = new QTableWidgetItem(tr("%n elements", "", _totalMusicFiles));
		totalFiles->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

		quickStartTableWidget->insertRow(0);
		quickStartTableWidget->setItem(0, 0, masterCheckBox);
		quickStartTableWidget->setItem(0, 1, new QTableWidgetItem(tr("%n folders", "", quickStartTableWidget->rowCount() - 1)));
		quickStartTableWidget->setItem(0, 2, totalFiles);

		quickStartApplyButton->setEnabled(true);

		_totalMusicFiles = 0;
	}
	_qsse->deleteLater();
	//_worker->deleteLater();
	sender()->deleteLater();
}

/** Insert a row with a checkbox with folder's name and the number of files in this folder. */
void QuickStart::insertRow(const QFileInfo &fileInfo, const int & musicFileNumber)
{
	// A subfolder is displayed with its number of files on the right
	QTableWidgetItem *checkBox = new QTableWidgetItem;
	checkBox->setFlags(checkBox->flags() | Qt::ItemIsUserCheckable);
	checkBox->setCheckState(Qt::Checked);

	QTableWidgetItem *musicSubFolderName = new QTableWidgetItem(fileInfo.baseName());
	musicSubFolderName->setData(Qt::UserRole, fileInfo.absoluteFilePath());

	QTableWidgetItem *musicSubFolderCount;
	if (musicFileNumber == 0) {
		musicSubFolderCount = new QTableWidgetItem(tr("empty folder"));
		checkBox->setCheckState(Qt::Unchecked);
	} else {
		musicSubFolderCount = new QTableWidgetItem(tr("%n elements", "", musicFileNumber));
	}
	musicSubFolderCount->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

	int rowCount = quickStartTableWidget->rowCount();
	quickStartTableWidget->insertRow(rowCount);
	quickStartTableWidget->setItem(rowCount, 0, checkBox);
	quickStartTableWidget->setItem(rowCount, 1, musicSubFolderName);
	quickStartTableWidget->setItem(rowCount, 2, musicSubFolderCount);

	_totalMusicFiles += musicFileNumber;
}
