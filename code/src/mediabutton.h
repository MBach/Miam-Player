#ifndef MEDIABUTTON_H
#define MEDIABUTTON_H

#include <QPushButton>

class MediaButton : public QPushButton
{
	Q_OBJECT
public:
	MediaButton(QWidget *parent = 0);
	
signals:
	void visibilityChanged(MediaButton *b, bool visible);
	
public slots:
	void setIconFromTheme(const QString &);
	void setSize(const int &);

	/// Override the QPushButton slot to add a write/read QSetting system
	void setVisible(bool visible);
};

#endif // MEDIABUTTON_H
