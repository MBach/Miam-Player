#ifndef MINIMODEWIDGET_H
#define MINIMODEWIDGET_H

#include "ui_minimode.h"

#include "abstractview.h"
#include "abstractmediaplayercontrol.h"

class MainWindow;

class MiniModeWidget : public AbstractView, public Ui::MiniMode
{
	Q_OBJECT
private:
	bool _startMoving;
	QPoint _pos, _globalPos;

public:
	explicit MiniModeWidget(MainWindow *mainWindow);

	virtual ~MiniModeWidget();

	void applyColorToStandardIcon(QAbstractButton *button);

	virtual bool eventFilter(QObject *obj, QEvent *e) override;

	virtual QSize sizeHint() const override;

	inline virtual ViewType type() const override { return VT_BuiltIn; }

	virtual bool viewProperty(Settings::ViewProperty vp) const override;

protected:
	virtual void closeEvent(QCloseEvent *) override;

	/** Redefined to be able to drag this widget on screen. */
	virtual void mouseMoveEvent(QMouseEvent *e) override;

	/** Redefined to be able to drag this widget on screen. */
	virtual void mouseReleaseEvent(QMouseEvent *e) override;

	virtual void mousePressEvent(QMouseEvent *e) override;

public slots:
	virtual void setViewProperty(Settings::ViewProperty vp, QVariant value) override;

private slots:
	void setPosition(qint64 pos, qint64 duration);

};

#endif // MINIMODEWIDGET_H
