#ifndef LIBRARYITEMMODEL_H
#define LIBRARYITEMMODEL_H

#include <QSet>
#include <model/genericdao.h>
#include <filehelper.h>
#include "miamitemmodel.h"
#include "separatoritem.h"
#include "miamlibrary_global.h"

#include "libraryfilterproxymodel.h"

/**
 * \brief		The LibraryItemModel class is used to cache information from the database, in order to increase performance.
 * \details
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMLIBRARY_LIBRARY LibraryItemModel : public MiamItemModel
{
	Q_OBJECT
private:
	LibraryFilterProxyModel *_proxy;

public:
	explicit LibraryItemModel(QObject *parent = nullptr);

	void clearCache();

	virtual QChar currentLetter(const QModelIndex &index) const override;

	virtual LibraryFilterProxyModel* proxy() const override;

	/** Rebuild the list of separators when one has changed grammatical articles in options. */
	void rebuildSeparators();

	void reset();

	inline QMultiHash<SeparatorItem*, QModelIndex> topLevelItems() const { return _topLevelItems; }

public slots:
	void cleanDanglingNodes();

	/** Find and insert a node in the hierarchy of items. */
	void insertNode(GenericDAO *node);
};

#endif // LIBRARYITEMMODEL_H
