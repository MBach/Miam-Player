#ifndef REFLECTOR_H
#define REFLECTOR_H

#include <QWidget>

/**
 * \brief		The Reflector Class is only designed to help the way one can customize colors.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class Reflector : public QWidget
{
	Q_OBJECT
private:
	/** The color used in the preview pane in the Customize Theme Dialog. */
	QColor _backgroundColor;

public:
	explicit Reflector(QWidget *parent = nullptr);

	/** Getter to the color used in the preview pane in the Customize Theme Dialog. */
	inline QColor color() const { return this->_backgroundColor; }

	QPalette::ColorRole colorRole() const;

	/** Setter to the color used in the preview pane in the Customize Theme Dialog. */
	void setColor(const QColor &color);

protected:
	/** Redefined to be able to reflect the color of the elements in the Customize Theme Dialog. */
	virtual void paintEvent(QPaintEvent *) override;
};

#endif // REFLECTOR_H
