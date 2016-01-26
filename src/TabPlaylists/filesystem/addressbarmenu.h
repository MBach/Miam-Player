#ifndef ADDRESSBARMENU_H
#define ADDRESSBARMENU_H

#include <QListWidget>

#include "addressbarbutton.h"

#include "miamtabplaylists_global.hpp"

/// Forward declaration
class AddressBar;

/**
 * \brief		The AddressBarMenu class is like a popup menu which displays subdirectories
 * \details		When this menu is opened, it shows every subfolders under the current highlighted button. Items are layed out in a list,
 *				with a scrollbar if lots of subdirectories are present. If one is moving the cursor from one button to another one, then the content
 *				of this menu is updated with the previous / next highlighted button. Folders are painted with the icon provided by your operating system,
 *				and disabled if it's not readable (like an external drive not ready nor mounted).
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMTABPLAYLISTS_LIBRARY AddressBarMenu : public QListWidget
{
	Q_OBJECT
	Q_ENUMS(SeparatorEnum)
private:
	QList<AddressBarButton*> subfolders;

	AddressBar *_addressBar;

	/** Used for root button. */
	bool _hasSeparator;

public:
	explicit AddressBarMenu(AddressBar *addressBar);

	enum SeparatorEnum { Separator = Qt::UserRole + 1};

	bool eventFilter(QObject *, QEvent *e);

	inline bool hasSeparator() const { return _hasSeparator; }

	void insertSeparator();

	void moveOrHide(const AddressBarButton *b);

protected:
	/** Redefined to force update the viewport. */
	virtual void mouseMoveEvent(QMouseEvent *e);

	/** Redefined to be able to display items with the current theme. */
	virtual void paintEvent(QPaintEvent *);

public slots:
	void clear();

	void show();
};

#endif // ADDRESSBARMENU_H
