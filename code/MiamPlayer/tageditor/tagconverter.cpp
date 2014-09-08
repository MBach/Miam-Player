#include "tagconverter.h"

#include "tageditortablewidget.h"
#include <QMouseEvent>

#include <QtDebug>

TagConverter::TagConverter(QPushButton *convertButton, TagEditorTableWidget *parent)
	: QDialog(parent, Qt::Popup), _convertButton(convertButton), _tagEditor(parent)
{
	setupUi(this);

	this->installEventFilter(this);

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

	connect(fileToTagApplyButton, &QPushButton::clicked, this, &TagConverter::applyPatternToColumns);
	connect(tagToFileApplyButton, &QPushButton::clicked, this, &TagConverter::applyPatternToFilenames);

	connect(_convertButton, &QPushButton::toggled, this, &TagConverter::setVisible);
}

void TagConverter::setVisible(bool b)
{
	if (b && _tagEditor->selectionModel()->hasSelection()) {
		this->autoGuessPatternFromFile();
	}
	if (b) {
		QPoint p = parentWidget()->mapToGlobal(_convertButton->pos());
		p.setX(p.x() - (width() - _convertButton->width()) / 2);
		this->move(p);
	}
	QDialog::setVisible(b);
}

bool TagConverter::eventFilter(QObject *obj, QEvent *event)
{
	// Close this popup when one is clicking outside this Dialog (otherwise this widget stays on top)
	if (obj == this && event->type() == QEvent::MouseButtonPress) {
		QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
		if (!rect().contains(mouseEvent->pos())) {
			this->setVisible(false);
			_convertButton->setChecked(false);
			return true;
		}
	}
	return QDialog::eventFilter(obj, event);
}

void TagConverter::applyPatternToColumns()
{
	QString pattern = this->generatePattern(fileToTagLineEdit);
	QRegularExpression re = this->generatePattern2(fileToTagLineEdit);
	//qDebug() << "pattern" << pattern << "to apply to columns";
	// Find columns to update
	//int nbOfPatterns = pattern.count(':');
	//QList<int> p;

	qDebug() << "regular expr" << re;

	foreach (QModelIndex index, _tagEditor->selectionModel()->selectedRows()) {
		QString filename = index.data().toString();
		/*int j = 0;
		for (int idx = 0; idx < pattern.size(); idx++) {
			if (pattern.at(idx) == ":") {

				TagEditorTableWidget::Columns column = static_cast<TagEditorTableWidget::Columns>(pattern.at(++idx).digitValue());
				//
				QString extract = "";
				switch (column) {
				case TagEditorTableWidget::COL_Track:
				case TagEditorTableWidget::COL_Year:
				case TagEditorTableWidget::COL_Disc:
					while (j < filename.size() && filename.at(j).isDigit()) {
						extract.append(filename.at(j));
						j++;
					}
					break;
				default:
					while (j < filename.size() && filename.at(j).isLetterOrNumber()) {
						extract.append(filename.at(j));
						j++;
					}
					break;
				}
				qDebug() << "for column" << column << ", extracted string" << extract;
			} else {
				QChar cPattern = pattern.at(idx);
				QChar cFilename = filename.at(j);
				if (cPattern != cFilename) {
					qDebug() << filename << ", at" << j << ":" << cFilename << "and" << cPattern;
					break;
				} else {
					j++;
				}
			}
		}*/
		qDebug() << filename <<  "match?" << re.match(filename);
	}
	this->setVisible(false);
	_convertButton->setChecked(false);
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
	this->setVisible(false);
	_convertButton->setChecked(false);
}

QString TagConverter::generatePattern(TagLineEdit *lineEdit) const
{
	qDebug() << Q_FUNC_INFO << lineEdit->text();
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

QRegularExpression TagConverter::generatePattern2(TagLineEdit *lineEdit) const
{
	QString text = lineEdit->text();
	text = text.replace(":", "_");
	qDebug() << Q_FUNC_INFO << text << QRegularExpression::escape(text);

	/// XXX code review needed
	// Proceed to substitutions in reverse order
	for (int i = lineEdit->tags().count() - 1; i >= 0; i--) {
		TagButton *tag = lineEdit->tags().at(i);
		QString characterClass;
		switch (tag->column()) {
		case TagEditorTableWidget::COL_Track:
		case TagEditorTableWidget::COL_Year:
		case TagEditorTableWidget::COL_Disc:
			characterClass = "\\d"; // Digits
			break;
		default:
			// What about non-ASCII characters? (éèêàâ...)
			//characterClass = "[A-Za-z]";
			characterClass = "\\S"; // Non-whitespace characters
			break;
		}

		QString substitution =  "(" + characterClass + ")+";
		text.replace(tag->position(), tag->spaceCount(), substitution);
	}
	qDebug() << "text" << text;
	return QRegularExpression(text);
}

QString TagConverter::autoGuessPatternFromFile() const
{
	//qDebug() << Q_FUNC_INFO;
	static const QStringList separators = QStringList() << "-" << "." << "_";
	return QString();
}
