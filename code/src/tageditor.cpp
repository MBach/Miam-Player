#include "tageditor.h"

TagEditor::TagEditor(QWidget *parent) :
	QWidget(parent)
{
	setupUi(this);

	connect(closeTagEditorButton, SIGNAL(clicked()), this, SLOT(close()));
}

void TagEditor::close()
{
	emit closeTagEditor(false);
	QWidget::close();
}

/** Delete all rows. */
void TagEditor::clear()
{
	while (tagEditorWidget->rowCount() > 0) {
		tagEditorWidget->removeRow(0);
	}
}
