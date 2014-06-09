#include "cover.h"

#include <QtDebug>

#include <QBuffer>
#include <QImage>
#include <QImageReader>
#include <QHash>

Cover::Cover(const QByteArray &byteArray, const QString &mimeType)
	: _hasChanged(false)
{
	_data = byteArray;
	_mimeType = mimeType;
	if (mimeType == "image/jpeg") {
		_format = "JPG";
	} else if (mimeType == "image/png") {
		_format = "PNG";
	} else {
		// default format is assumed to be jpg
		_format = "JPG";
	}
}

/** Constructor used when loading pictures directly from the filesystem (drag & drop or with the context menu). */
Cover::Cover(const QString &fileName)
{
	if (!fileName.isEmpty()) {
		// QImage is faster than QPixmap for I/O ops
		QImage image(fileName);
		if (!image.isNull()) {
			QString format = QImageReader::imageFormat(fileName);
			QBuffer buffer(&_data);
			if (buffer.open(QIODevice::WriteOnly)) {
				if (image.save(&buffer, format.toStdString().data())) {
					if (format == "jpeg") {
						_format = "JPG";
						_mimeType = "image/jpeg";
					} else if (format == "png") {
						_format = "PNG";
						_mimeType = "image/png";
					}
					_hasChanged = true;
				}
				buffer.close();
			}
		}
	}
}
