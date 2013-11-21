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

	connect(quickStartTableWidget, &QTableWidget::cellClicked, this, &QuickStart::checkRow);
	connect(quickStartTableWidget, &QTableWidget::itemClicked, [=] (QTableWidgetItem *i) {
		if (i->column() == 0) {
			this->checkRow(i->row(), i->column());
		}
	});

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
		_worker = new QThread();
		_qsse = new QuickStartSearchEngine();
		_qsse->moveToThread(_worker);
		connect(_qsse, &QuickStartSearchEngine::folderScanned, this, &QuickStart::insertRow);
		connect(_worker, &QThread::started, _qsse, &QuickStartSearchEngine::doSearch);
		connect(_worker, &QThread::finished, this, &QuickStart::insertFirstRow);
		_worker->start();
	}
	//this->setStyleSheet("QuickStart#" + objectName() + " { border-top: 0; border-left: #ACACAC; border-bottom: #ACACAC;
	//border-right: #ACACAC; border-width: 1; border-style: solid; background-color: white; }");
}

void QuickStart::checkRow(int row, int)
{
	if (row == 0) {
		if (quickStartTableWidget->item(0, 0)->checkState() == Qt::Checked) {
			for (int r = 1; r < quickStartTableWidget->rowCount(); r++) {
				quickStartTableWidget->item(r, 0)->setCheckState(Qt::Unchecked);
			}
		} else {
			for (int r = 0; r < quickStartTableWidget->rowCount(); r++) {
				quickStartTableWidget->item(r, 0)->setCheckState(Qt::Checked);
			}
		}
	} else {
		if (quickStartTableWidget->item(row, 0)->checkState() == Qt::Checked) {
			quickStartTableWidget->item(row, 0)->setCheckState(Qt::Unchecked);
		} else {
			quickStartTableWidget->item(row, 0)->setCheckState(Qt::Checked);
		}
	}

	// Each time one clicks on a row, it's necessary to check if the apply button has to va enabled
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
		masterCheckBox->setCheckState(Qt::Checked);

		QTableWidgetItem *totalFiles = new QTableWidgetItem(tr("%1 elements").arg(_totalMusicFiles));
		totalFiles->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

		quickStartTableWidget->insertRow(0);
		quickStartTableWidget->setItem(0, 0, masterCheckBox);
		quickStartTableWidget->setItem(0, 1, new QTableWidgetItem(tr("%1 folders").arg(quickStartTableWidget->rowCount() - 1)));
		quickStartTableWidget->setItem(0, 2, totalFiles);

		quickStartApplyButton->setEnabled(true);

		_totalMusicFiles = 0;
	}
	_qsse->deleteLater();
	_worker->deleteLater();
}

/** Insert a row with a checkbox with folder's name and the number of files in this folder. */
void QuickStart::insertRow(const QFileInfo &fileInfo, int musicFileNumber)
{
	QString elements = tr("%1 elements");

	// A subfolder is displayed with its number of files on the right
	QTableWidgetItem *checkBox = new QTableWidgetItem;
	checkBox->setFlags(checkBox->flags() | Qt::ItemIsUserCheckable);
	checkBox->setCheckState(Qt::Checked);

	QTableWidgetItem *musicSubFolderName = new QTableWidgetItem(fileInfo.baseName());
	musicSubFolderName->setData(Qt::UserRole, fileInfo.absoluteFilePath());

	QTableWidgetItem *musicSubFolderCount = new QTableWidgetItem(elements.arg(musicFileNumber));
	musicSubFolderCount->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

	int rowCount = quickStartTableWidget->rowCount();
	quickStartTableWidget->insertRow(rowCount);
	quickStartTableWidget->setItem(rowCount, 0, checkBox);
	quickStartTableWidget->setItem(rowCount, 1, musicSubFolderName);
	quickStartTableWidget->setItem(rowCount, 2, musicSubFolderCount);

	_totalMusicFiles += musicFileNumber;
}
