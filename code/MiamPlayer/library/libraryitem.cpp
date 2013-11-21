#include "libraryitem.h"
#include "librarymodel.h"
#include "settings.h"

#include <QtDebug>

LibraryItem::LibraryItem(const QString &text) :
	QStandardItem(text)
{
	setFont(Settings::getInstance()->font(Settings::LIBRARY));
	QString n = text.normalized(QString::NormalizationForm_KD).toLower().remove(QRegExp("[^0-9a-z\\s]"));
	if (n.isEmpty()) {
		n = " ";
	}
	this->setData(n, NormalizedString);
}
