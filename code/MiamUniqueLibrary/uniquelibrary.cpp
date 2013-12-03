#include "uniquelibrary.h"

#include "ui_uniquelibrary.h"

#include "flowlayout.h"

#include <QPushButton>

UniqueLibrary::UniqueLibrary(QWidget *parent) :
	QWidget(parent), ui(new Ui::UniqueLibrary)
{
	ui->setupUi(this);
	_flowLayout = new FlowLayout();
	ui->scrollArea->setWidgetResizable(true);
	ui->scrollArea->widget()->setLayout(_flowLayout);

	_maxH = 100;
	_maxW = 100;
	connect(ui->buttonsNumberSpinBox, SIGNAL(valueChanged(int)), this, SLOT(changeNumberOfButtons(int)));
	connect(ui->hButtonsSpinBox, SIGNAL(valueChanged(int)), this, SLOT(changeHeightOfButtons(int)));
	connect(ui->wButtonsSpinBox, SIGNAL(valueChanged(int)), this, SLOT(changeWidthOfButtons(int)));
}

void UniqueLibrary::changeNumberOfButtons(int max)
{
	while (QLayoutItem* item = _flowLayout->takeAt(0)) {
		delete item->widget();
		delete item;
	}

	for (int i = 1; i <= max; i++) {
		QPushButton *w = new QPushButton();
		w->setStyleSheet("QPushButton { background: yellow } ");
		w->setMinimumHeight(_maxH);
		w->setMaximumHeight(_maxH);
		w->setMinimumWidth(_maxW);
		w->setMaximumWidth(_maxW);
		ui->scrollArea->widget()->layout()->addWidget(w);
	}
}

void UniqueLibrary::changeHeightOfButtons(int max)
{
	_maxH = max;
	QList<QPushButton*> ws = ui->scrollArea->findChildren<QPushButton*>();
	foreach (QPushButton *w, ws) {
		w->setMinimumHeight(_maxH);
		w->setMaximumHeight(_maxH);
	}
}

void UniqueLibrary::changeWidthOfButtons(int max)
{
	_maxW = max;
	QList<QPushButton*> ws = ui->scrollArea->findChildren<QPushButton*>();
	foreach (QPushButton *w, ws) {
		w->setMinimumWidth(_maxW);
		w->setMaximumWidth(_maxW);
	}
}
