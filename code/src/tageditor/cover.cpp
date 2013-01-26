#include "cover.h"

Cover::Cover()
{
	_format = 0;
}

Cover::Cover(const QByteArray &data, const char *mimeType)
{
	_data = data;
	_mimeType = mimeType;
	if (strcmp(mimeType, "image/jpeg") == 0) {
		_format = "JPG";
	} else if (strcmp(mimeType, "image/png") == 0) {
		_format = "PNG";
	} else {
		// default
		_format = "JPG";
	}
}
