#ifndef EQUALIZERDIALOG_H
#define EQUALIZERDIALOG_H

#include <QDialog>

#include "ui_equalizerdialog.h"

/**
 * \brief		The EqualizerDialog class display a small 10-band equalizer.
 * \details		One can also load a preset from VLC and apply them to the current track. It's possible to interact on each
 *				band individually, including the the pre-amplification.
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class EqualizerDialog : public QDialog, public Ui::EqualizerDialog
{
	Q_OBJECT
public:
	explicit EqualizerDialog(QWidget *parent = 0);

	virtual ~EqualizerDialog();

	virtual void setVisible(bool visible) override;

protected:
	virtual bool eventFilter(QObject *obj, QEvent *ev);

private:
	/** Create a preset icon from VLC's presets. */
	QIcon createPresetIcon(uint presetIndex);

	/** Toggle the equalizer on a track. */
	void toggle(bool b);

private slots:
	/** Apply a preset and update sliders. */
	void applySelectedPreset();
};

#endif // EQUALIZERDIALOG_H
