#include "equalizerdalog.h"
#include "mediaplayer.h"

//#include <vlc-qt/MediaPlayer.h>
//#include <vlc-qt/Equalizer.h>
#include "settingsprivate.h"

#include <QPainter>
#include <QScrollBar>
#include <QWheelEvent>

QStringList EqualizerDialog::presets = (QStringList() << QT_TR_NOOP("Flat") << QT_TR_NOOP("Classical") << QT_TR_NOOP("Club")
	<< QT_TR_NOOP("Dance") << QT_TR_NOOP("Full bass") << QT_TR_NOOP("Full bass and treble") << QT_TR_NOOP("Full treble")
	<< QT_TR_NOOP("Headphones") << QT_TR_NOOP("Large Hall") << QT_TR_NOOP("Live") << QT_TR_NOOP("Party") << QT_TR_NOOP("Pop")
	<< QT_TR_NOOP("Reggae") << QT_TR_NOOP("Rock") << QT_TR_NOOP("Ska") << QT_TR_NOOP("Soft") << QT_TR_NOOP("Soft rock") << QT_TR_NOOP("Techno"));

EqualizerDialog::EqualizerDialog(QWidget *parent) :
	QDialog(parent, Qt::Tool)
{
	setupUi(this);
	this->setAttribute(Qt::WA_DeleteOnClose, true);
//	MediaPlayer *mediaPlayer = MediaPlayer::instance();
//	VlcEqualizer *equalizer = mediaPlayer->vlcMediaPlayer()->equalizer();

	// Connect each slider to VLC's equalizer
	for (QSlider *slider : findChildren<QSlider*>()) {
		QLabel *label = findChild<QLabel*>(slider->objectName().replace("slider", "label"));
		connect(slider, &QSlider::valueChanged, this, [=](int value) {
			float f = value / 10.0f;
			label->setText(QString::number(f, 'f', 1) + " db");
//			if (slider == preamp_slider) {
//				equalizer->setPreamplification(f);
//			} else {
//				int bandIndex = slider->objectName().mid(5, 2).toInt() - 1;
//				equalizer->setAmplificationForBandAt(f, bandIndex);
//			}
		});
	}

	// Fill Combo box with preset list
	/*for (uint i = 0; i < equalizer->presetCount(); i++) {
		QString preset = QApplication::translate("EqualizerDialog", equalizer->presetNameAt(i).toStdString().data());
		QListWidgetItem *item = new QListWidgetItem(this->createPresetIcon(i), preset);
		presetList->addItem(item);
	}*/

	connect(toggleEqualizer, &QCheckBox::toggled, this, &EqualizerDialog::toggle);
	//connect(presetList, &QListWidget::currentRowChanged, equalizer, &VlcEqualizer::loadFromPreset);
	//connect(equalizer, &VlcEqualizer::presetLoaded, this, &EqualizerDialog::applySelectedPreset);

	presetList->installEventFilter(this);

	SettingsPrivate *s = SettingsPrivate::instance();
	if (!s->value("equalizer/preampValue").isNull()) {

		int row = s->value("equalizer/lastPresetUsed", -1).toInt();
		if (row != -1) {
			presetList->setCurrentRow(row);
			toggleEqualizer->setChecked(true);
		}

		float preamp = s->value("equalizer/preampValue").toFloat();
		//equalizer->setPreamplification(preamp);
		preamp_slider->setValue(preamp * 10.0f);

		QMap<QString, QVariant> values = s->value("equalizer/bandValues").toMap();

		QMapIterator<QString, QVariant> it(values);
		while (it.hasNext()) {
			it.next();
			QSlider *slider = findChild<QSlider*>(it.key());
			if (slider) {
				float f = it.value().toFloat();
				slider->setValue(f * 10.0f);

				//int bandIndex = slider->objectName().mid(5, 2).toInt() - 1;
				//equalizer->setAmplificationForBandAt(f, bandIndex);
			}
		}
	}
}

EqualizerDialog::~EqualizerDialog()
{
	bool b = toggleEqualizer->isChecked();
	SettingsPrivate *settings = SettingsPrivate::instance();
	if (b) {
		float preampValue = 0;
		QMap<QString, QVariant> values;
		for (QSlider *slider : findChildren<QSlider*>()) {
			float f = slider->value() / 10.0f;
			if (slider == preamp_slider) {
				preampValue = f;
			} else {
				values.insert(slider->objectName(), QVariant::fromValue<float>(f));
			}
		}
		settings->setValue("equalizer/preampValue", preampValue);
		settings->setValue("equalizer/bandValues", values);
		settings->setValue("equalizer/lastPresetUsed", presetList->currentRow());
	} else {
		settings->remove("equalizer/preampValue");
		settings->remove("equalizer/bandValues");
		settings->remove("equalizer/lastPresetUsed");
	}
}

bool EqualizerDialog::eventFilter(QObject *obj, QEvent *ev)
{
	bool b;
	if (ev->type() == QEvent::Wheel) {
		b = false;
		QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(ev);
		if (wheelEvent->delta() < 0) {
			presetList->horizontalScrollBar()->triggerAction(QAbstractSlider::SliderSingleStepAdd);
		} else {
			presetList->horizontalScrollBar()->triggerAction(QAbstractSlider::SliderSingleStepSub);
		}
	} else {
		b = QDialog::eventFilter(obj, ev);
	}
	return b;
}

void EqualizerDialog::setVisible(bool visible)
{
	if (visible) {
		auto mediaPlayer = MediaPlayer::instance();
		auto playlist = mediaPlayer->playlist();
		if (playlist) {
			QMediaContent mc = playlist->currentMedia();
			toggleEqualizer->setEnabled(mc.canonicalUrl().isLocalFile());
		} else {
			this->toggle(false);
			toggleEqualizer->setEnabled(false);
		}
	}
	QDialog::setVisible(visible);
}

/** Create a preset icon from VLC's presets. */
QIcon EqualizerDialog::createPresetIcon(uint presetIndex)
{
	// Values in [-20.0 db ; 20.0 db] range are converted into [0.000 ; 1.000]
	// VLC has 18 presets
	static double defaultVLCPresetList[][10] = {
		{0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500},
		{0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.325, 0.325, 0.325, 0.260},
		{0.500, 0.500, 0.700, 0.638, 0.638, 0.638, 0.580, 0.500, 0.500, 0.500},
		{0.740, 0.675, 0.560, 0.500, 0.500, 0.363, 0.325, 0.325, 0.500, 0.500},
		{0.300, 0.740, 0.740, 0.638, 0.540, 0.400, 0.300, 0.243, 0.223, 0.223},
		{0.675, 0.638, 0.500, 0.325, 0.380, 0.540, 0.700, 0.778, 0.800, 0.800},
		{0.260, 0.260, 0.260, 0.400, 0.560, 0.778, 0.900, 0.900, 0.900, 0.918},
		{0.620, 0.778, 0.638, 0.420, 0.440, 0.540, 0.620, 0.740, 0.820, 0.858},
		{0.758, 0.758, 0.638, 0.638, 0.500, 0.380, 0.380, 0.380, 0.500, 0.500},
		{0.380, 0.500, 0.600, 0.638, 0.638, 0.638, 0.600, 0.560, 0.560, 0.560},
		{0.675, 0.675, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.675, 0.675},
		{0.460, 0.620, 0.675, 0.700, 0.638, 0.500, 0.440, 0.440, 0.460, 0.460},
		{0.500, 0.500, 0.500, 0.363, 0.500, 0.660, 0.660, 0.500, 0.500, 0.500},
		{0.700, 0.620, 0.363, 0.300, 0.420, 0.600, 0.720, 0.778, 0.778, 0.778},
		{0.440, 0.380, 0.400, 0.500, 0.600, 0.638, 0.720, 0.740, 0.778, 0.740},
		{0.620, 0.540, 0.500, 0.440, 0.500, 0.600, 0.700, 0.740, 0.778, 0.800},
		{0.600, 0.600, 0.560, 0.500, 0.400, 0.363, 0.420, 0.500, 0.560, 0.720},
		{0.700, 0.638, 0.500, 0.363, 0.380, 0.500, 0.700, 0.740, 0.740, 0.720}
	};

	auto defaultVLCPreset = defaultVLCPresetList[presetIndex];

	// Create a painter on a QPixmap to be able to paint over it
	QSize iconSize(32, 32);
	QPixmap pixmap(iconSize);
	QPainter painter(&pixmap);
	QPalette palette = QApplication::palette();

	qreal xAxis = iconSize.height();
	qreal yAxis = iconSize.width();

	// Fill background with light gradient
	QLinearGradient gradient(0, 0, 0, yAxis);
	gradient.setColorAt(0.0, palette.base().color());
	gradient.setColorAt(1.0, palette.window().color());
	painter.fillRect(QRect(QPoint(0, 0), iconSize), gradient);

	// Create points on a curve from the preset
	QPointF start(0, yAxis - defaultVLCPreset[0] * yAxis);
	QPainterPath curvePath;
	curvePath.moveTo(0, 0.5 * yAxis);
	curvePath.lineTo(start);
	for (int t = 1; t < 10; t++) {
		QPointF to;
		to.setX(t * xAxis / 10);
		to.setY(yAxis - defaultVLCPreset[t] * yAxis);
		curvePath.lineTo(to);
	}
	curvePath.lineTo(xAxis, 0.5 * yAxis);

	// Draw the outline of the curve
	painter.setRenderHint(QPainter::Antialiasing, true);
	QPen pen(Qt::black);
	pen.setWidthF(0.5);
	painter.setPen(pen);
	painter.drawPath(curvePath);

	// Fill the curve
	QLinearGradient blueRedGradient(0, 0, 0, yAxis);
	blueRedGradient.setColorAt(0.0, palette.highlight().color());
	blueRedGradient.setColorAt(0.5, palette.mid().color());
	blueRedGradient.setColorAt(1.0, Qt::red);
	painter.fillPath(curvePath, blueRedGradient);
	painter.setRenderHint(QPainter::Antialiasing, false);

	// Draw frame rectangle
	painter.setPen(palette.mid().color());
	painter.drawRect(0, 0, 31, 31);

	return QIcon(pixmap);
}

/** Toggle the equalizer on a track. */
void EqualizerDialog::toggle(bool b)
{
	for (QSlider *slider : findChildren<QSlider*>()) {
		slider->setEnabled(b);
	}
	for (QLabel *label : findChildren<QLabel*>()) {
		label->setEnabled(b);
	}
	presetList->setEnabled(b);
	//auto equalizer = MediaPlayer::instance()->vlcMediaPlayer()->equalizer();
	//if (equalizer) {
	//	equalizer->setEnabled(b);
	//}
}

/** Apply a preset and update sliders. */
void EqualizerDialog::applySelectedPreset()
{
	/*auto equalizer = MediaPlayer::instance()->vlcMediaPlayer()->equalizer();
	for (QSlider *slider : findChildren<QSlider*>()) {
		QLabel *label = findChild<QLabel*>(slider->objectName().replace("slider", "label"));
		if (slider == preamp_slider) {
			slider->setValue(equalizer->preamplification() * 10);
			label->setText(QString::number(equalizer->preamplification(), 'f', 1) + " db");
		} else {
			int bandIndex = slider->objectName().mid(5, 2).toInt() - 1;
			slider->setValue(equalizer->amplificationForBandAt(bandIndex) * 10);
			int i = equalizer->amplificationForBandAt(bandIndex) * 10;
			label->setText(QString::number(i / 10.0f, 'f', 1) + " db");
		}
	}*/
}
