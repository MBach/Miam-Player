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

	connect(fileToTagApplyButton, &QPushButton::clicked, this, [=]() {
		QString pattern = this->generatePattern(fileToTagLineEdit);
	});

	connect(tagToFileApplyButton, &QPushButton::clicked, this, &TagConverter::applyPatternToFilenames);
}

void TagConverter::setVisible(bool b)
{
	if (b && _tagEditor->selectionModel()->hasSelection()) {
		this->autoGuessPatternFromFile();
	}
	QDialog::setVisible(b);
}

void TagConverter::applyPatternToFilenames()
{
	QString pattern = this->generatePattern(tagToFileLineEdit);

	int column = 0;
	foreach (QModelIndex index, _tagEditor->selectionModel()->selectedRows()) {
		QString text = "";
		for (int c = 0; c < pattern.size(); c++) {
			if (pattern.at(c) == ":") {
				/// XXX: Working for < 10 columns in TagEditor!
				int column = pattern.at(++c).digitValue();
				text += _tagEditor->item(index.row(), column)->text();
			} else {
				text += pattern.at(c);
			}
		}
		QString item = _tagEditor->item(index.row(), 0)->text();
		int extension = item.lastIndexOf(".");
		text.append(item.mid(extension));
		_tagEditor->updateCellData(index.row(), column, text);
	}
}

QString TagConverter::generatePattern(TagLineEdit *lineEdit) const
{
	qDebug() << "lineEdit" << lineEdit->text();
	QString pattern = lineEdit->text();
	pattern = pattern.replace(":", "_");
	/// XXX code review needed
	// Proceed to substitutions in reverse order
	for (int i = lineEdit->tags().count() - 1; i >= 0; i--) {
		TagButton *tag = lineEdit->tags().at(i);
		QString substitution =  ":" + QString::number(tag->column());
		pattern.replace(tag->position(), tag->spaceCount(), substitution);
	}
	qDebug() << "pattern" << pattern;
	return pattern;
}

QString TagConverter::autoGuessPatternFromFile() const
{
	qDebug() << Q_FUNC_INFO;
	static const QStringList separators = QStringList() << "-" << "." << "_";
	return QString();
}
