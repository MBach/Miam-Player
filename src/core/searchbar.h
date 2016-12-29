#ifndef SEARCHBAR
#define SEARCHBAR

#include <QLineEdit>
#include <QTimer>

#include "miamcore_global.h"

/**
 * \brief		The SearchBar class
 * \details
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY SearchBar : public QLineEdit
{
	Q_OBJECT
public:
	SearchBar(QWidget *parent = nullptr) : QLineEdit(parent)
	{
		// Do not start search when one is typing. Add a 300ms delay after the last key pressed.
		QTimer *timer = new QTimer(this);
		timer->setSingleShot(true);
		connect(this, &QLineEdit::textEdited, this, [=]() {	timer->start(300); });
		connect(timer, &QTimer::timeout, this, [=]() { emit aboutToStartSearch(this->text()); });
	}

	inline virtual ~SearchBar() {}

signals:
	void aboutToStartSearch(const QString &text);
};

#endif // SEARCHBAR

