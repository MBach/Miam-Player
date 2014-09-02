#ifndef CIRCLEPROGRESSBAR_H
#define CIRCLEPROGRESSBAR_H

#include <QProgressBar>

/// XXX: DELETE or not?
/// It is not really following Windows 8 UI guidelines. What about other OS?
/// Moreover, one cannot interact with Library when full rescan is happening
/// It should be replace by a thin horizontal progress bar in the bottom when library is populating!
class CircleProgressBar : public QProgressBar
{
	Q_OBJECT

private:
	bool isTransparentCenter;
	qreal startAngle;

	QRectF innerRect;
	QRectF outerRect;

	// The gray background on Windows 7
	QRadialGradient grayRadialGradient;

	// The green groove on Windows 7
	QRadialGradient grooveRadialGradient;

public:
	CircleProgressBar(QWidget *parent = 0);

	inline void setTransparentCenter(bool value) { isTransparentCenter = value; }
	inline void setStartAngle(qreal startAngle) { this->startAngle = startAngle; }

protected:
	void paintEvent(QPaintEvent *event);
};

#endif // CIRCLEPROGRESSBAR_H
