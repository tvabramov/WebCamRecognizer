#include <QCameraInfo>
#include <QMessageBox>
#include <QtWidgets>
#include <QThread>
#if !defined(QT_NO_OPENGL)
#include <QGLWidget>
#endif
#include "capturablevideosurface.h"
#include "mobilenetssdrecognizer.h"
#include "recognitionitem.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

Q_DECLARE_METATYPE(QCameraInfo)

MainWindow::MainWindow(QWidget *_parent) :
    QMainWindow(_parent),
    ui(new Ui::MainWindow),
	mCamera(nullptr)
{
    ui->setupUi(this);

	// Viewfinder Surface and GraphicView

	{
		mSurface = new CapturableVideoSurface(QRectF(QPointF(0, 0), QPointF(ui->graphicsViewFinder->width(),
		ui->graphicsViewFinder->height())));

		QGraphicsScene *scene = new QGraphicsScene(this);
		scene->addItem(mSurface);
		ui->graphicsViewFinder->setScene(scene);

		#if !defined(QT_NO_OPENGL)
		ui->graphicsViewFinder->setViewport(new QGLWidget);
		#endif
	}

	// Recognition GraphicItem and GraphicView

	{
		mRecItem = new RecognitionItem(QRectF(QPointF(0, 0), QPointF(ui->graphicsViewRecognition->width(), ui->graphicsViewRecognition->height())),
			ui->sliderConfidenceThreshold->value());
		QGraphicsScene *scene = new QGraphicsScene;
		scene->addItem(mRecItem);
		ui->graphicsViewRecognition->setScene(scene);

		#if !defined(QT_NO_OPENGL)
		ui->graphicsViewRecognition->setViewport(new QGLWidget);
		#endif
	}

	connect(ui->actionRecognize, &QAction::triggered, mSurface, &CapturableVideoSurface::querySnapshot);
	connect(ui->sliderConfidenceThreshold, &QSlider::valueChanged, mRecItem, &RecognitionItem::setConfThresholdProc);
	connect(ui->actionInfiniteRecognition, &QAction::triggered, this, &MainWindow::onInfiniteRecognitionToggled);
	connect(ui->actionAboutProgram, &QAction::triggered, this, &MainWindow::onAboutProgram);
	connect(ui->actionAboutQt, &QAction::triggered, qApp, &QApplication::aboutQt);

	// ----------- Camera selection actions -----------

	QActionGroup *camerasGroup = new QActionGroup(this);
	camerasGroup->setExclusive(true);
	for (const QCameraInfo &cameraInfo : QCameraInfo::availableCameras()) {

		QAction *videoDeviceAction = new QAction(cameraInfo.description(), camerasGroup);
		videoDeviceAction->setCheckable(true);
		videoDeviceAction->setData(QVariant::fromValue(cameraInfo));
		if (cameraInfo == QCameraInfo::defaultCamera())
			videoDeviceAction->setChecked(true);

		ui->menuDevices->addAction(videoDeviceAction);
	}

	connect(camerasGroup, &QActionGroup::triggered, this, &MainWindow::onCameraSelected);

	connect(ui->actionToggleCamera, &QAction::toggled, this, &MainWindow::onCameraToggled);
	connect(ui->actionToggleLock, &QAction::toggled, this, &MainWindow::onLockToggled);

	// Recognizer

	mRecognizer = new MobileNetSSDRecognizer;
	mRecognizerThread = new QThread(this);
	mRecognizer->moveToThread(mRecognizerThread);

	connect(mRecognizerThread, &QThread::finished, mRecognizer, &RecognitionItem::deleteLater);
	connect(mSurface, &CapturableVideoSurface::newSnapshot, mRecognizer, &MobileNetSSDRecognizer::recognize);
	connect(mRecognizer, &MobileNetSSDRecognizer::newRecognition, this, &MainWindow::onNewRecognition);
	connect(mRecognizer, &MobileNetSSDRecognizer::newRecognition, mRecItem, &RecognitionItem::setRecognition);

	mRecognizerThread->start();

	// Tool bar

	ui->mainToolBar->addAction(ui->actionToggleCamera);
	ui->mainToolBar->addAction(ui->actionToggleLock);
	ui->mainToolBar->addAction(ui->actionRecognize);
	ui->mainToolBar->addAction(ui->actionInfiniteRecognition);

	// For consistency

	onCameraSelected(camerasGroup->checkedAction());
}

MainWindow::~MainWindow()
{
	mRecognizerThread->quit();
	mRecognizerThread->wait();

	delete ui;
}

void MainWindow::setCamera(const QCameraInfo &_cameraInfo)
{
	if (mCamera) {

		mCamera->stop();
		mCamera->deleteLater();
	}

	mCamera = new QCamera(_cameraInfo, this);

	connect(mCamera, &QCamera::stateChanged, this, &MainWindow::onCameraStateChanged);
	connect(mCamera, static_cast<void(QCamera::*)(QCamera::Error)>(&QCamera::error),
		this, &MainWindow::onCameraErrorOccurred);
	connect(ui->sliderExposureCompensation, &QSlider::valueChanged, this, &MainWindow::onExposureCompensationSetted);
	connect(mCamera, static_cast<void(QCamera::*)(QCamera::LockStatus, QCamera::LockChangeReason)>(&QCamera::lockStatusChanged),
		this, &MainWindow::onLockStatusChanged);

	mCamera->setViewfinder(mSurface);

	ui->actionToggleLock->setEnabled(mCamera->supportedLocks() != QCamera::NoLock && mCamera->state() == QCamera::ActiveState);
	ui->actionRecognize->setEnabled(!ui->actionInfiniteRecognition->isChecked() && mCamera->state() == QCamera::ActiveState);
	ui->actionInfiniteRecognition->setEnabled(mCamera->state() == QCamera::ActiveState);

	ui->groupBoxCamera->setTitle(tr("Viewfinder [%1]").arg(_cameraInfo.description()));

	// For consistency

	onCameraStateChanged(mCamera->state());
	onLockStatusChanged(mCamera->lockStatus(), QCamera::UserRequest);
	onExposureCompensationSetted(ui->sliderExposureCompensation->value());

	if (mCamera->isAvailable()) {

		ui->actionToggleCamera->setEnabled(true);
		mCamera->start();
	}
	else {

		ui->actionToggleCamera->setEnabled(false);
		QMessageBox::warning(this, tr("Camera Error"), tr("Camera is not available"));
	}
}

void MainWindow::keyPressEvent(QKeyEvent *_event)
{
	if (_event->isAutoRepeat())
		return;

	switch (_event->key())
	{
		case Qt::Key_CameraFocus:
			mCamera->searchAndLock();
			_event->accept();
			break;
		default:
			QMainWindow::keyPressEvent(_event);
	}
}

void MainWindow::keyReleaseEvent(QKeyEvent *_event)
{
	if (_event->isAutoRepeat())
		return;

	switch (_event->key())
	{
		case Qt::Key_CameraFocus:
			mCamera->unlock();
			break;
		default:
			QMainWindow::keyReleaseEvent(_event);
	}
}

void MainWindow::onCameraSelected(QAction *_action)
{
	setCamera(_action ? qvariant_cast<QCameraInfo>(_action->data()) : QCameraInfo::defaultCamera());
}

void MainWindow::onCameraToggled(bool _on)
{	
	return _on ? mCamera->start() : mCamera->stop();
}

void MainWindow::onLockToggled(bool _on)
{
	return _on ? mCamera->searchAndLock() : mCamera->unlock();
}

void MainWindow::onInfiniteRecognitionToggled(bool _on)
{
	if (_on) {

		ui->actionRecognize->setEnabled(false);
		mSurface->querySnapshot();
	} else {

		ui->actionRecognize->setEnabled(mCamera->state() == QCamera::ActiveState);
	}
}

void MainWindow::onCameraStateChanged(QCamera::State _state)
{
	switch (_state) {

		case QCamera::ActiveState:
			ui->actionToggleCamera->blockSignals(true);
			ui->actionToggleCamera->setChecked(true);
			ui->actionToggleCamera->setText(tr("Camera Off"));
			ui->actionToggleCamera->setToolTip("Switch the Camera Off");
			ui->actionToggleCamera->setStatusTip("Switch the Camera Off");
			ui->actionToggleCamera->setWhatsThis("Switch the Camera Off");
			ui->actionToggleCamera->blockSignals(false);

			ui->actionToggleLock->setEnabled(mCamera->supportedLocks() != QCamera::NoLock);
			ui->actionRecognize->setEnabled(!ui->actionInfiniteRecognition->isChecked());
			ui->actionInfiniteRecognition->setEnabled(true);

			break;

		default:
			ui->actionToggleCamera->blockSignals(true);
			ui->actionToggleCamera->setChecked(false);
			ui->actionToggleCamera->setText(tr("Camera On"));
			ui->actionToggleCamera->setToolTip("Switch the Camera On");
			ui->actionToggleCamera->setStatusTip("Switch the Camera On");
			ui->actionToggleCamera->setWhatsThis("Switch the Camera On");
			ui->actionToggleCamera->blockSignals(false);

			ui->actionToggleLock->setEnabled(false);
			ui->actionRecognize->setEnabled(false);
			ui->actionInfiniteRecognition->setEnabled(false);
	}
}

void MainWindow::onLockStatusChanged(QCamera::LockStatus _status, QCamera::LockChangeReason /*_reason*/)
{
	switch (_status) {

		case QCamera::Unlocked:
			ui->actionToggleLock->blockSignals(true);
			ui->actionToggleLock->setChecked(false);
			ui->actionToggleLock->setText(tr("Focus and Lock Camera"));
			ui->actionToggleLock->setToolTip("Focus and Lock Camera");
			ui->actionToggleLock->setStatusTip("Focus and Lock Camera");
			ui->actionToggleLock->setWhatsThis("Focus and Lock Camera");
			ui->actionToggleLock->blockSignals(false);

			break;

		default:
			ui->actionToggleLock->blockSignals(true);
			ui->actionToggleLock->setChecked(true);
			ui->actionToggleLock->setText(tr("Unlock Camera"));
			ui->actionToggleLock->setToolTip("Unlock Camera");
			ui->actionToggleLock->setStatusTip("Unlock Camera");
			ui->actionToggleLock->setWhatsThis("Unlock Camera");
			ui->actionToggleLock->blockSignals(false);
	}
}

void MainWindow::onCameraErrorOccurred(QCamera::Error /*_err*/)
{
	QMessageBox::warning(this, tr("Camera Error"), mCamera->errorString());
}

void MainWindow::onExposureCompensationSetted(int _index)
{
	mCamera->exposure()->setExposureCompensation(_index * 0.5);
}

void MainWindow::onNewRecognition(QSharedPointer<Recognition> /*_rec*/)
{
	if (ui->actionInfiniteRecognition->isChecked())
		QTimer::singleShot(0, mSurface, SLOT(querySnapshot()));
}

void MainWindow::onAboutProgram()
{
	QString s = "<font color=red><b><big>" + tr("WebCamRecognizer") + "</big></b></font><br/>";
	s += "<font color=green><big>" + tr("Prorgram version: ") + PROGRAMVERSION + "</big></font><br/>";
	s += "<font color=black>" + tr("OpenCV version: ") + CV_VERSION + "</font><br/>";
	s += "<font color=black>" + tr("Qt version: ") + QT_VERSION_STR + "</font><br/>";
	s += "<hr>" + tr("Application for chairs recognition and counting on webcam snapshots") + "<br /><br />";

	QMessageBox::about(this, "About Program", s);
}
