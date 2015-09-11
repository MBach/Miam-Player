#include "miamitemdelegate.h"

#include <settingsprivate.h>

qreal MiamItemDelegate::_iconOpacity = 1.0;

MiamItemDelegate::MiamItemDelegate(QSortFilterProxyModel *proxy)
	: QStyledItemDelegate(proxy), _proxy(proxy), _timer(new QTimer(this)), _coverSize(48)
{
	_libraryModel = qobject_cast<QStandardItemModel*>(_proxy->sourceModel());
	_showCovers = SettingsPrivate::instance()->isCoversEnabled();
	_timer->setTimerType(Qt::PreciseTimer);
	_timer->setInterval(10);
}

