#ifndef CIRCLEPROGRESSBAR_H
#define CIRCLEPROGRESSBAR_H

#include <QProgressBar>

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
