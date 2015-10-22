#ifndef IMAGEUTILS_H
#define IMAGEUTILS_H

#include <QImage>
#include "miamcore_global.h"

class MIAMCORE_LIBRARY ImageUtils
{
private:
	ImageUtils();

public:
	// Thanks StackOverflow for this algorithm (works like a charm without any changes)
	static QImage blurred(const QImage& image, const QRect& rect, int radius, bool alphaOnly = false);
};

#endif // IMAGEUTILS_H
