#include "libraryfilterlineedit.h"

//#include "settings.h"

LibraryFilterLineEdit::LibraryFilterLineEdit(QWidget *parent) :
	QLineEdit(parent)
{
	/// FIXME
	//QString styleSheet = Settings::getInstance()->styleSheet(this);
	//styleSheet = styleSheet.arg(clearButton->sizeHint().width() + frameWidth + 1);
	//this->setStyleSheet(styleSheet);

	// Remove text when clicked
	this->setClearButtonEnabled(true);
	connect(this, &QLineEdit::cursorPositionChanged, [=](int, int newP){
		if (newP == 0) {
			emit textEdited(QString());
		}
	});
}
