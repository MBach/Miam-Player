#include "libraryfilterlineedit.h"

#include "mainwindow.h"
#include "settings.h"

LibraryFilterLineEdit::LibraryFilterLineEdit(QWidget *parent) :
	QLineEdit(parent)
{
	QPixmap pixmap(":/config/closeButton");
	clearButton = new QToolButton(this);
	clearButton->setIcon(QIcon(pixmap));
	clearButton->setIconSize(pixmap.size());
	clearButton->setCursor(Qt::ArrowCursor);
	clearButton->setStyleSheet("QToolButton { border: none; padding: 0px; }");

	int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
	QString styleSheet = Settings::getInstance()->styleSheet(this);
	styleSheet = styleSheet.arg(clearButton->sizeHint().width() + frameWidth + 1);
	this->setStyleSheet(styleSheet);

	QSize msz = minimumSizeHint();
	setMinimumSize(qMax(msz.width(), clearButton->sizeHint().height() + frameWidth * 2 + 2),
				   qMax(msz.height(), clearButton->sizeHint().height() + frameWidth * 2 + 2));

	// Remove text when clicked
	connect(clearButton, SIGNAL(clicked()), this, SLOT(clear()));
	connect(this, SIGNAL(textChanged(const QString&)), this, SLOT(updateClearButtonStatus(const QString&)));

	updateClearButtonStatus();
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
	// Explicit call to remove filter in the library (not emitted with setText())
	emit textEdited(0);
	clearFocus();
}

/** Show or hide the clear button. */
void LibraryFilterLineEdit::updateClearButtonStatus(const QString& searchText)
{
	clearButton->setHidden(searchText.isEmpty());
}
