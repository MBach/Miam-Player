#ifndef CUSTOMIZETHEMETAGLINEEDIT_H
#define CUSTOMIZETHEMETAGLINEEDIT_H

#include "taglineedit.h"

/**
 * \brief		The CustomizeThemeTagLineEdit class
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class CustomizeThemeTagLineEdit : public TagLineEdit
{
	Q_OBJECT
private:
	QTimer *_timerTag;

public:
	explicit CustomizeThemeTagLineEdit(QWidget *parent = 0);

	virtual bool eventFilter(QObject *obj, QEvent *event) override;

protected slots:
	virtual void closeTagButton(TagButton *t) override;

	//virtual bool createTag() override;

signals:
	void taglistHasChanged(const QStringList &tagslist);
};

#endif // CUSTOMIZETHEMETAGLINEEDIT_H
