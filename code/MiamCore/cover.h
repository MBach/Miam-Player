#ifndef COVER_H
#define COVER_H

#include <QString>

#include "miamcore_global.h"

class MIAMCORE_LIBRARY Cover
{
private:
	/** Like "image/jpeg" (for TagLib). */
	QString _mimeType;

	/** Like "JPG" (for QClasses). */
	QString _format;

	QByteArray _data;

	bool _hasChanged;

public:
	Cover(const QByteArray &byteArray, const QString &mimeType);

	/** Constructor used when loading pictures directly from the filesystem (drag & drop or with the context menu). */
	Cover(const QString &fileName = QString());

	inline const char* mimeType() const { return _mimeType.toStdString().data(); }

	inline const QByteArray byteArray() const { return _data; }

	inline const char* format() { return _format.toStdString().data(); }

	inline bool hasChanged() const { return _hasChanged && !_data.isEmpty(); }

	inline void setChanged(bool changed) { this->_hasChanged = changed; }
};

#endif // COVER_H
