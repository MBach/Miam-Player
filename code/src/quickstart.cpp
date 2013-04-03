#include "quickstart.h"

#include "columnutils.h"
#include "filehelper.h"
#include "nofocusitemdelegate.h"

#include <QDirIterator>
#include <QStandardPaths>

#include <QtDebug>

QuickStart::QuickStart(QWidget *parent) :
	QWidget(parent)
{
	setupUi(this);

	quickStartTableWidget->setItemDelegate(new NoFocusItemDelegate(this));

	connect(quickStartTableWidget, &QTableWidget::cellClicked, this, &QuickStart::checkRow);
	connect(quickStartTableWidget, &QTableWidget::itemClicked, [=] (QTableWidgetItem *i) {
		if (i->column() == 0) {
			this->checkRow(i->row(), i->column());
		}
	});

	//this->setStyleSheet("#quickStart { border-left: #ACACAC; border-bottom: #ACACAC; border-right: #ACACAC; border-width: 1; border-style: solid; background-color: white; }");
	//this->setStyleSheet("#QuickStart { border-left: #ACACAC; border-bottom: #ACACAC; border-right: #ACACAC; border-width: 1; border-style: solid; background-color: white; }");
}

/** The first time the player is launched, this function will scan for multimedia files. */
void QuickStart::setVisible(bool b)
{
	if (b) {
		while (quickStartTableWidget->rowCount() > 0) {
			quickStartTableWidget->removeRow(0);
		}
		QString userMusicPath = QStandardPaths::standardLocations(QStandardPaths::MusicLocation).first();
		if (userMusicPath.isEmpty()) {
			quickStartGroupBox->hide();
		} else {
			QDir musicDir(userMusicPath);
			int totalMusicFiles = 0;
			// For every subfolder in the user's music path, a quick test on multimedia files is done
			foreach (QFileInfo fileInfo, musicDir.entryInfoList(QDir::Dirs|QDir::NoDotAndDotDot, QDir::Name)) {
				int rowCount = quickStartTableWidget->rowCount();
				quickStartTableWidget->insertRow(rowCount);

				// Files are excluded if the player can't read them!
				QString elements = tr("%1 elements");
				QStringList filters;
				foreach (QString filter, FileHelper::suffixes()) {
					filters.append("*." + filter);
				}
				QDirIterator it(fileInfo.absoluteFilePath(), filters, QDir::Files, QDirIterator::Subdirectories);
				int musicFiles = 0;
				while (it.hasNext()) {
					it.next();
					musicFiles++;
				}

				// A subfolder is displayed with its number of files on the right
				QTableWidgetItem *checkBox = new QTableWidgetItem;
				checkBox->setFlags(checkBox->flags() | Qt::ItemIsUserCheckable);
				checkBox->setCheckState(Qt::Checked);

				QTableWidgetItem *musicSubFolderName = new QTableWidgetItem(fileInfo.baseName());
				musicSubFolderName->setData(Qt::UserRole, fileInfo.absoluteFilePath());

				QTableWidgetItem *musicSubFolderCount = new QTableWidgetItem(elements.arg(musicFiles));
				musicSubFolderCount->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

				quickStartTableWidget->setItem(rowCount, 0, checkBox);
				quickStartTableWidget->setItem(rowCount, 1, musicSubFolderName);
				quickStartTableWidget->setItem(rowCount, 2, musicSubFolderCount);
				totalMusicFiles += musicFiles;
			}

			if (totalMusicFiles == 0) {
				quickStartGroupBox->hide();
			} else {
				QList<int> ratios = (QList<int>() << 0 << 3 << 2);
				ColumnUtils::resizeColumns(quickStartTableWidget, ratios);

				quickStartTableWidget->insertRow(0);

				QTableWidgetItem *masterCheckBox = new QTableWidgetItem;
				masterCheckBox->setFlags(masterCheckBox->flags() | Qt::ItemIsTristate | Qt::ItemIsUserCheckable);
				masterCheckBox->setCheckState(Qt::Checked);

				QTableWidgetItem *totalFiles = new QTableWidgetItem(tr("%1 elements").arg(totalMusicFiles));
				totalFiles->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

				quickStartTableWidget->setItem(0, 0, masterCheckBox);
				quickStartTableWidget->setItem(0, 1, new QTableWidgetItem(tr("%1 folders").arg(quickStartTableWidget->rowCount() - 1)));
				quickStartTableWidget->setItem(0, 2, totalFiles);
			}
		}
	}
	QWidget::setVisible(b);
}

void QuickStart::resizeEvent(QResizeEvent *)
{
	QList<int> ratios(QList<int>() << 0 << 3 << 2);
	ColumnUtils::resizeColumns(quickStartTableWidget, ratios);
}

void QuickStart::checkRow(int row, int)
{
	qDebug() << "checkRow" << row;
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
