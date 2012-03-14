#ifndef STARRATING_H
#define STARRATING_H

#include <QMetaType>
#include <QPointF>
#include <QVector>

class StarRating
{
private:
	static int maxStars;
	int stars;

public:
	enum EditMode { Editable, ReadOnly };

	StarRating(int starCount = 0);

	void paint(QPainter *painter, const QRect *rect, const QPalette &palette, EditMode mode) const;

	inline int starCount() const { return stars; }

	static inline int max() { return maxStars; }

	void setStarCount(int starCount) { stars = starCount; }
	QSize sizeHint() const;
};

Q_DECLARE_METATYPE(StarRating)

#endif // STARRATING_H
