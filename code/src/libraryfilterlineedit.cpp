#include "libraryfilterlineedit.h"

#include "mainwindow.h"

LibraryFilterLineEdit::LibraryFilterLineEdit(QWidget *parent) :
    QLineEdit(parent)
{
	defaultSearchText = tr("Search...");
	setText(defaultSearchText);

	QPixmap pixmap(":/config/closeButton");
	clearButton = new QToolButton(this);
	clearButton->setIcon(QIcon(pixmap));
	clearButton->setIconSize(pixmap.size());
	clearButton->setCursor(Qt::ArrowCursor);
	clearButton->setStyleSheet("QToolButton { border: none; padding: 0px; }");

	int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
	setStyleSheet(QString("QLineEdit { padding-right: %1px; } ").arg(clearButton->sizeHint().width() + frameWidth + 1));
	QSize msz = minimumSizeHint();
	setMinimumSize(qMax(msz.width(), clearButton->sizeHint().height() + frameWidth * 2 + 2),
				   qMax(msz.height(), clearButton->sizeHint().height() + frameWidth * 2 + 2));

	// Remove text when clicked
	connect(clearButton, SIGNAL(clicked()), this, SLOT(clear()));
	connect(this, SIGNAL(textChanged(const QString&)), this, SLOT(updateClearButtonStatus(const QString&)));

	updateClearButtonStatus();
}

/** Reimplemented from QLineEdit::focusInEvent(). */
void LibraryFilterLineEdit::focusInEvent(QFocusEvent *event)
{
	// This block is useful when a new tranlator is installed (= the language is not EN anymore).
	if (text() == tr(defaultSearchText.toStdString().data())) {
		defaultSearchText = tr(defaultSearchText.toStdString().data());
	}
	if (text() == defaultSearchText) {
		QLineEdit::clear();
		QFont oldFont = font();
		oldFont.setItalic(false);
		setFont(oldFont);
	}
	QLineEdit::focusInEvent(event);
}

/** Reimplemented from QLineEdit::focusOutEvent(). */
void LibraryFilterLineEdit::focusOutEvent(QFocusEvent *event)
{
	if (text().isEmpty()) {
		QFont oldFont = font();
		oldFont.setItalic(true);
		setFont(oldFont);
		setText(defaultSearchText);
	}
	QLineEdit::focusOutEvent(event);
}

/** Keep the clear button on the right. */
void LibraryFilterLineEdit::resizeEvent(QResizeEvent *)
{
	QSize sz = clearButton->sizeHint();
	int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
	clearButton->move(rect().right() - frameWidth - sz.width(),
					  (rect().bottom() + 1 - sz.height()) / 2);
}

/** Redefined to add custom behaviour. */
void LibraryFilterLineEdit::clear()
{
	QLineEdit::clear();
	QFont oldFont = font();
	oldFont.setItalic(true);
	setFont(oldFont);
	setText(defaultSearchText);

	// Explicit call to remove filter in the library (not emitted with setText())
	emit textEdited(0);
	clearFocus();
}

/** Show or hide the clear button. */
void LibraryFilterLineEdit::updateClearButtonStatus(const QString& searchText)
{
	clearButton->setHidden(searchText.isEmpty() || this->text() == tr("Search..."));
}
