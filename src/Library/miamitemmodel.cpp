#include "miamitemmodel.h"
#include "albumitem.h"

#include <settingsprivate.h>

#include <QtDebug>

MiamItemModel::MiamItemModel(QObject *parent)
	: QStandardItemModel(parent)
{

}

SeparatorItem *MiamItemModel::insertSeparator(const QStandardItem *node)
{
	// Items are grouped every ten years in this particular case
	switch (SettingsPrivate::instance()->insertPolicy()) {
	case SettingsPrivate::IP_Years: {
		int year = node->text().toInt();
		if (year == 0) {
			return nullptr;
		}
		QString yearStr = QString::number(year - year % 10);
		if (_letters.contains(yearStr)) {
			return _letters.value(yearStr);
		} else {
			SeparatorItem *separator = new SeparatorItem(yearStr);
			separator->setData(yearStr, Miam::DF_NormalizedString);
			invisibleRootItem()->appendRow(separator);
			_letters.insert(yearStr, separator);
			return separator;
		}
		break;
	}
	// Other types of hierarchy, separators are built from letters
	default:
		QString c;
		if (node->data(Miam::DF_CustomDisplayText).toString().isEmpty()) {
			c = node->text().left(1).normalized(QString::NormalizationForm_KD).toUpper().remove(QRegExp("[^A-Z\\s]"));
		} else {
			QString reorderedText = node->data(Miam::DF_CustomDisplayText).toString();
			c = reorderedText.left(1).normalized(QString::NormalizationForm_KD).toUpper().remove(QRegExp("[^A-Z\\s]"));
		}
		QString letter;
		bool topLevelLetter = false;
		if (c.contains(QRegExp("\\w"))) {
			letter = c;
		} else {
			letter = tr("Various");
			topLevelLetter = true;
		}
		if (_letters.contains(letter)) {
			return _letters.value(letter);
		} else {
			SeparatorItem *separator = new SeparatorItem(letter);
			if (topLevelLetter) {
				separator->setData("0", Miam::DF_NormalizedString);
			} else {
				separator->setData(letter.toLower(), Miam::DF_NormalizedString);
			}
			invisibleRootItem()->appendRow(separator);
			_letters.insert(letter, separator);
			return separator;
		}
	}
	return nullptr;
}

/** Recursively remove node and its parent if the latter has no more children. */
void MiamItemModel::removeNode(const QModelIndex &node)
{
	QModelIndex parent = node.parent();
	this->removeRow(node.row(), parent);
	if (!hasChildren(parent)) {
		this->removeNode(parent);
	}
}

void MiamItemModel::updateNode(GenericDAO *node)
{
	uint h = node->hash();
	/// Why do I have to update asynchronously this node? Can I just not fill all the information in the Database class?
	if (AlbumItem *album = static_cast<AlbumItem*>(_hash.value(h))) {
		AlbumDAO *dao = qobject_cast<AlbumDAO*>(node);
		album->setData(dao->year(), Miam::DF_Year);
		album->setData(dao->cover(), Miam::DF_CoverPath);
		album->setData(dao->icon(), Miam::DF_IconPath);
		album->setData(!dao->icon().isEmpty(), Miam::DF_IsRemote);
	}/* else if (TrackItem *track = static_cast<TrackItem*>(_hash.value(h))) {
		TrackDAO *dao = qobject_cast<TrackDAO*>(node);
	} else if (ArtistItem *artist = static_cast<ArtistItem*>(_hash.value(h))) {
		ArtistDAO *dao = qobject_cast<ArtistDAO*>(node);
	}*/
}
