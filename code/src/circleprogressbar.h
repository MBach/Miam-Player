#ifndef CIRCLEPROGRESSBAR_H
#define CIRCLEPROGRESSBAR_H

#include <QProgressBar>

class CircleProgressBar : public QProgressBar
{
	Q_OBJECT

private:
	bool transparentCenter;
	qreal startAngle;

public:
	CircleProgressBar(QWidget *parent = 0);

	inline void setTransparentCenter(bool value) { transparentCenter = value; }
	inline void setStartAngle(qreal startAngle) { this->startAngle = startAngle; }

protected:
	void paintEvent(QPaintEvent *event);
};

#endif // CIRCLEPROGRESSBAR_H
