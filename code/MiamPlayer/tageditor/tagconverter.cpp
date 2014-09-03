#include "tagconverter.h"

#include "tageditortablewidget.h"

#include <QtDebug>

TagConverter::TagConverter(TagEditorTableWidget *parent)
	: QDialog(parent, Qt::Popup), _tagEditor(parent)
{
	setupUi(this);

	foreach (QToolButton *toolButton, tagToFileGroupBox->findChildren<QToolButton*>()) {
		connect(toolButton, &QToolButton::clicked, this, [=]() {
			tagToFileLineEdit->addTag(toolButton->text(), toolButton->property("column").toInt());
		});
	}
	foreach (QToolButton *toolButton, fileToTagGroupBox->findChildren<QToolButton*>()) {
		connect(toolButton, &QToolButton::clicked, this, [=]() {
			fileToTagLineEdit->addTag(toolButton->text(), toolButton->property("column").toInt());
		});
	}

	connect(tagToFilePreviewButton, &QPushButton::clicked, this, [=]() {
		QString pattern = this->generatePattern(tagToFileLineEdit);
		qDebug() << "tagToFilePreviewButton" << pattern;
	});

	connect(tagToFileApplyButton, &QPushButton::clicked, this, [=]() {
		QString pattern = this->generatePattern(tagToFileLineEdit);
		qDebug() << "tagToFileApplyButton" << pattern;
	});

	connect(fileToTagPreviewButton, &QPushButton::clicked, this, [=]() {
		QString pattern = this->generatePattern(fileToTagLineEdit);
		qDebug() << "fileToTagPreviewButton" << pattern;
	});

	connect(fileToTagApplyButton, &QPushButton::clicked, this, [=]() {
		QString pattern = this->generatePattern(fileToTagLineEdit);
		qDebug() << "fileToTagApplyButton" << pattern;

		//pattern.indexOf(":")

		int column = 0;
		foreach (QModelIndex index, _tagEditor->selectionModel()->selectedRows()) {
			/*for (int c = 0; c < cols; c++) {

			}*/
			QString text = pattern;
			QString item = _tagEditor->item(index.row(), 0)->text();
			int extension = item.lastIndexOf(".");
			text.append(item.mid(extension));
			_tagEditor->updateCellData(index.row(), column, text);
		}
	});
}

QString TagConverter::generatePattern(TagLineEdit *lineEdit) const
{
	QString pattern = lineEdit->text();
	pattern = pattern.replace(":", " ");
	// Proceed to substitutions in reverse order
	for (int i = lineEdit->tags().count() - 1; i >= 0; i--) {
		TagButton *tag = lineEdit->tags().at(i);
		pattern.replace(tag->position(), tag->spaceCount(), ":" + QString::number(tag->column()));
	}
	return pattern;
}
