#include "minimodewidget.h"

#include "model/sqldatabase.h"
#include "settings.h"
#include "mainwindow.h"

#include <QDesktopWidget>
#include <QMainWindow>
#include <QMouseEvent>
#include <QPainter>

#include <QtDebug>

MiniModeWidget::MiniModeWidget(MainWindow *mainWindow)
	: AbstractView(mainWindow->currentView()->mediaPlayerControl(), nullptr)
	, _startMoving(false)
	, _pos(0, 0)
{
	this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
	setupUi(this);

	// Media buttons
	previous->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
	playPause->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
	stop->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
	next->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));

	auto settings = Settings::instance();
	QMap<QString, QVariant> shortcutMap = settings->shortcuts();
	previous->setShortcut(QKeySequence(shortcutMap.value("skipBackward").toString()));
	playPause->setShortcut(QKeySequence(shortcutMap.value("play").toString()));
	stop->setShortcut(QKeySequence(shortcutMap.value("stop").toString()));
	next->setShortcut(QKeySequence(shortcutMap.value("skipForward").toString()));

	// Window management
	minimize->setIcon(style()->standardIcon(QStyle::SP_TitleBarMinButton));
	restore->setIcon(style()->standardIcon(QStyle::SP_TitleBarMaxButton));
	closeButton->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));

	for (QPushButton *b : findChildren<QPushButton*>()) {
		applyColorToStandardIcon(b);
	}

	auto mediaPlayerControl = mainWindow->currentView()->mediaPlayerControl();
	slider->setMediaPlayer(mediaPlayerControl->mediaPlayer());
	slider->installEventFilter(this);
	this->installEventFilter(this);

	// Multimedia actions
	connect(previous, &QPushButton::clicked, mediaPlayerControl, &AbstractMediaPlayerControl::skipBackward);
	connect(playPause, &QPushButton::clicked, mediaPlayerControl, &AbstractMediaPlayerControl::togglePlayback);
	connect(stop, &QPushButton::clicked, mediaPlayerControl->mediaPlayer(), &MediaPlayer::stop);
	connect(next, &QPushButton::clicked, mediaPlayerControl, &AbstractMediaPlayerControl::skipForward);

	// Windows actions
	connect(minimize, &QPushButton::clicked, this, &MiniModeWidget::showMinimized);
	connect(restore, &QPushButton::clicked, this, [=]() {
		this->setVisible(false);
		mainWindow->setVisible(true);
		this->deleteLater();
	});
	connect(closeButton, &QPushButton::clicked, &QApplication::quit);

	connect(mediaPlayerControl->mediaPlayer(), &MediaPlayer::currentMediaChanged, this, [=](const QString &uri) {
		SqlDatabase db;
		TrackDAO track = db.selectTrackByURI(uri);
		currentTrack->setText(track.trackNumber().append(" - ").append(track.title()));
	});

	connect(mediaPlayerControl->mediaPlayer(), &MediaPlayer::stateChanged, this, [=](QMediaPlayer::State state) {
		switch (state) {
		case QMediaPlayer::StoppedState:
			// Reset the label to 0
			time->setTime(0, mediaPlayerControl->mediaPlayer()->duration());
			playPause->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
			break;
		case QMediaPlayer::PlayingState:
			playPause->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
			break;
		case QMediaPlayer::PausedState:
			playPause->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
			break;
		}
	});

	connect(mediaPlayerControl->mediaPlayer(), &MediaPlayer::positionChanged, time, &TimeLabel::setTime);
	connect(mediaPlayerControl->mediaPlayer(), &MediaPlayer::positionChanged, slider, &MiniSlider::setPosition);
}

MiniModeWidget::~MiniModeWidget()
{
	disconnect(mediaPlayerControl()->mediaPlayer(), 0, this, 0);
}

bool MiniModeWidget::eventFilter(QObject *obj, QEvent *e)
{
	if (obj == slider && e->type() == QEvent::MouseMove) {
		_mediaPlayerControl->mediaPlayer()->seek((qreal) slider->value() / (qreal)100.0);
	} else if (obj == slider && e->type() == QEvent::MouseButtonRelease) {
		_mediaPlayerControl->mediaPlayer()->setMute(false);
	} else if (obj == slider && e->type() == QEvent::MouseButtonPress) {
		_mediaPlayerControl->mediaPlayer()->setMute(true);
		_mediaPlayerControl->mediaPlayer()->seek((qreal) slider->value() / (qreal)100.0);
	}
	return QObject::eventFilter(obj, e);
}

QSize MiniModeWidget::sizeHint() const
{
	return QSize(275, 30);
}

bool MiniModeWidget::viewProperty(Settings::ViewProperty vp) const
{
	switch (vp) {
	case Settings::VP_MediaControls:
	case Settings::VP_SearchArea:
	case Settings::VP_HideMenuBar:
	case Settings::VP_HasTracksToDisplay:
	case Settings::VP_OwnWindow:
		return true;
	case Settings::VP_PlaylistFeature:
		return mediaPlayerControl()->mediaPlayer()->playlist() != nullptr;
	default:
		return false;
	}
}

void MiniModeWidget::closeEvent(QCloseEvent *)
{
	QCoreApplication::instance()->quit();
}

/** Redefined to be able to drag this widget on screen. */
void MiniModeWidget::mouseMoveEvent(QMouseEvent *e)
{
	static int _OFFSET = 15;
	if (_startMoving) {
		/// TODO multiple screens
		// Detect when one has changed from one screen to another
		if (e->pos().x() < 0 || e->pos().y() < 0) {
			//qDebug() << "todo multiple screens";
		}
		const QRect screen = QApplication::desktop()->screenGeometry();
		// Top edge screen
		if (frameGeometry().top() - screen.top() <= _OFFSET && (e->globalPos().y() - _pos.y()) <= _OFFSET) {
			//qDebug() << "top edge" << frameGeometry() << _pos << e->globalPos();
			move(e->globalPos().x() - _pos.x(), 0);
		} else if (screen.right() - frameGeometry().right() <= _OFFSET && (_globalPos.x() - e->globalPos().x()) <= _OFFSET) { // Right edge screen
			//qDebug() << "right edge" << frameGeometry() << _pos << e->globalPos();
			move(screen.right() - frameGeometry().width() + 1, e->globalPos().y() - _pos.y());
			_globalPos = e->globalPos();
		} else if (screen.bottom() - frameGeometry().bottom() <= _OFFSET && (_pos.y() - e->globalPos().y()) <= _OFFSET) { // Bottom edge screen
			//qDebug() << "bottom edge" << frameGeometry() << _pos << e->globalPos();
			move(e->globalPos().x() - _pos.x(), 0);
		} else if (frameGeometry().left() - screen.left() <= _OFFSET && (e->globalPos().x() - _pos.x()) <= _OFFSET) { // Left edge screen
			//qDebug() << "left edge" << frameGeometry() << _pos << e->globalPos();
			move(0, e->globalPos().y() - _pos.y());
		} else { // Inside the screen
			//qDebug() << "inside screen" << frameGeometry();
			// Substract the click in the middle of this widget because we don't want to move() to (0,0)
			move(e->globalPos().x() - _pos.x(), e->globalPos().y() - _pos.y());
		}
	}
	AbstractView::mouseMoveEvent(e);
}

/** Redefined to be able to drag this widget on screen. */
void MiniModeWidget::mouseReleaseEvent(QMouseEvent *e)
{
	// Reset the position
	_startMoving = false;
	_pos = QPoint();
	AbstractView::mouseReleaseEvent(e);
}

void MiniModeWidget::mousePressEvent(QMouseEvent *e)
{
	_startMoving = true;
	// Keep a reference from one's cursor
	_pos = e->pos();
	_globalPos = e->globalPos();
	//}
	AbstractView::mousePressEvent(e);
}

void MiniModeWidget::setViewProperty(Settings::ViewProperty vp, QVariant value)
{
	Q_UNUSED(vp)
	Q_UNUSED(value)
}

void MiniModeWidget::setPosition(qint64 pos, qint64 duration)
{
	if (duration > 0) {
		time->setTime(pos, duration);
		if (duration > 0) {
			slider->setValue(pos / duration * 100);
		}
	}
}

void MiniModeWidget::applyColorToStandardIcon(QAbstractButton *button)
{
	QPixmap sourcePixmap(8, 8);
	QPixmap destinationPixmap = button->icon().pixmap(8, 8);
	QPixmap resultPixmap = QPixmap(destinationPixmap);

	QPainter painter(&resultPixmap);
	painter.setCompositionMode(QPainter::CompositionMode_Source);
	painter.fillRect(resultPixmap.rect(), Qt::transparent);

	painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
	painter.drawPixmap(0, 0, destinationPixmap);

	painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
	sourcePixmap.fill(QColor(236, 206, 122));
	title->setStyleSheet("color: white;");
	currentTrack->setStyleSheet("color: #00ff00;");
	time->setStyleSheet("color: #00ff00;");
	this->setStyleSheet("background: #39395A;");
	slider->setStyleSheet("QSlider::groove:horizontal  { \
								  border: 1px solid #999999; \
								  border-top-color: #000010; \
								  border-right-color: #5A6B7B; \
								  border-bottom-color: #5A6B7B; \
								  border-left-color: #000010; \
								  background: #39395A; \
								  margin: 0; \
							  } \
							  QSlider::handle:horizontal  { \
								  background: 1px solid #ECCE7A; \
								  border-radius: 1px; \
								  margin-top: -1px; \
								  margin-bottom: -1px; \
								  width: 3px; \
							  }");
	painter.drawPixmap(0, 0, sourcePixmap);

	painter.setCompositionMode(QPainter::CompositionMode_DestinationOver);
	painter.fillRect(resultPixmap.rect(), Qt::transparent);
	painter.end();

	button->setIcon(resultPixmap);
}
