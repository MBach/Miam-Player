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
	QList<int> columns;

	//QRegularExpression re = this->generatePattern2(fileToTagLineEdit);
	///
	QString text = fileToTagLineEdit->text();
	text = text.replace(':', '_');

	// Remove and convert spaces according to columns
	for (int i = fileToTagLineEdit->tags().count() - 1; i >= 0; i--) {
		TagButton *tag = fileToTagLineEdit->tags().at(i);
		QString substitution = ':' + QString::number(tag->column());
		columns.prepend(tag->column());
		text.replace(tag->position(), tag->spaceCount(), substitution);
	}

	QString pattern = "^";
	QString characterClass;
	// Depending on which detected columns, choose a sub-regex
	for (int i = 0; i < text.size(); i++) {
		QChar c = text.at(i);
		if (c == ':') {
			c = text.at(++i);
			switch (c.digitValue()) {
			case TagEditorTableWidget::COL_Track:
			case TagEditorTableWidget::COL_Year:
			case TagEditorTableWidget::COL_Disc:
				characterClass = "[\\d]+"; // Digits
				break;
			default:
				characterClass = "[\\w ']+";	// Words, digits, and spaces
				break;
			}
			pattern += "(" + characterClass + ")";
		} else {
			pattern += QRegularExpression::escape(text.at(i));
		}
	}
	pattern += "$";
	QRegularExpression re(pattern, QRegularExpression::UseUnicodePropertiesOption);
	///

	qDebug() << "regular expr" << re;

	foreach (QModelIndex index, _tagEditor->selectionModel()->selectedRows()) {
		QString filename = index.data().toString();
		filename = filename.left(filename.lastIndexOf("."));
		QRegularExpressionMatch m = re.match(filename);
		if (!m.hasMatch()) {
			continue;
		}
		// The implicit capturing group with index 0 captures the result of the whole match.
		for (int i = 0; i < re.captureCount(); i++) {
			_tagEditor->updateCellData(index.row(), columns.at(i), m.captured(i + 1));
		}
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
			if (pattern.at(c) == ':') {
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
	pattern = pattern.replace(':', '_');

	/// XXX code review needed
	// Proceed to substitutions in reverse order
	for (int i = lineEdit->tags().count() - 1; i >= 0; i--) {
		TagButton *tag = lineEdit->tags().at(i);
		QString substitution =  ':' + QString::number(tag->column());
		pattern.replace(tag->position(), tag->spaceCount(), substitution);
	}
	qDebug() << "pattern" << pattern;
	return pattern;
}

QString TagConverter::autoGuessPatternFromFile() const
{
	//qDebug() << Q_FUNC_INFO;
	static const QStringList separators = QStringList() << "-" << "." << "_";
	return QString();
}
