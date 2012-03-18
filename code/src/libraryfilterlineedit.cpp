#include "libraryfilterlineedit.h"

#include "mainwindow.h"

LibraryFilterLineEdit::LibraryFilterLineEdit(QWidget *parent) :
    QLineEdit(parent)
{
	defaultSearchText = tr("Search...");
	setText(defaultSearchText);
}


void LibraryFilterLineEdit::focusInEvent(QFocusEvent *event)
{
	// This block is useful when a new tranlator is installed (= the language is not EN anymore).
	if (text() == tr(defaultSearchText.toStdString().data())) {
		defaultSearchText = tr(defaultSearchText.toStdString().data());
	}
	if (text() == defaultSearchText) {
		clear();
		QFont oldFont = font();
		oldFont.setItalic(false);
		setFont(oldFont);
	}
	QLineEdit::focusInEvent(event);
}

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
