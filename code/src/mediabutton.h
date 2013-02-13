#ifndef MEDIABUTTON_H
#define MEDIABUTTON_H

#include <QPushButton>

/**
Class for buttons like "Play", "Stop", etc.
*/
class MediaButton : public QPushButton
{
	Q_OBJECT
public:
	MediaButton(QWidget *parent = 0);

	/** Redefined to load custom icons saved in settings. */
	void setIcon(const QIcon &, bool toggled = false);

	/** Redefined to set shortcuts from settings at startup. */
	void setObjectName(const QString &);

signals:
	/** Hide or show buttons from options. */
	void visibilityChanged(MediaButton *b, bool visible);

public slots:
	/** Load an icon from a chosen theme in options. */
	void setIconFromTheme(const QString &);

	/** Change the size of icons from the options. */
	void setSize(const int &);

	/** Override the QPushButton slot to add a write/read QSetting system. */
	void setVisible(bool visible);
};

#endif // MEDIABUTTON_H
