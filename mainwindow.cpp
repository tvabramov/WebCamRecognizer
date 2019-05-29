#include <QCameraInfo>
#include <QMessageBox>
#include <QtWidgets>

#include "mainwindow.h"
#include "ui_mainwindow.h"

Q_DECLARE_METATYPE(QCameraInfo)

MainWindow::MainWindow(QWidget *_parent) :
    QMainWindow(_parent),
    ui(new Ui::MainWindow),
	mCamera(new QCamera(QCameraInfo::defaultCamera(), this)),
	mCapturer(new QCameraImageCapture(mCamera, this))
{
    ui->setupUi(this);

	// Camera

	connect(mCamera, &QCamera::stateChanged, this, &MainWindow::onCameraStateChanged);
	connect(mCamera, static_cast<void(QCamera::*)(QCamera::Error)>(&QCamera::error),
		this, &MainWindow::onCameraErrorOccurred);

	mCamera->setViewfinder(ui->viewFinder);

	onCameraStateChanged(mCamera->state());

	// Capturer

	connect(mCapturer, &QCameraImageCapture::readyForCaptureChanged, this, &MainWindow::onReadyForCaptureChanged);
	connect(mCapturer, &QCameraImageCapture::imageCaptured, this, &MainWindow::onImageCaptured);
	connect(mCapturer, static_cast<void(QCameraImageCapture::*)(int, QCameraImageCapture::Error, const QString &)>(&QCameraImageCapture::error),
		this, &MainWindow::onCapturerErrorOccurred);

	onReadyForCaptureChanged(mCapturer->isReadyForCapture());

	// Other

	connect(ui->buttonGetShapshot, &QPushButton::clicked, mCapturer, [this](){
		mCapturer->capture();
	});

	mCamera->start();
}

MainWindow::~MainWindow()
{
	delete ui;
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
		case Qt::Key_Camera:
			if (mCamera->captureMode() == QCamera::CaptureStillImage)
				mCapturer->capture();
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

void MainWindow::onImageCaptured(int /*_requestId*/, const QImage& _img)
{
	QImage scaledImage = _img.scaled(ui->labelSnapshot->size(),
		Qt::KeepAspectRatio, Qt::SmoothTransformation);

	ui->labelSnapshot->setPixmap(QPixmap::fromImage(scaledImage));
}

void MainWindow::onCapturerErrorOccurred(int /*_id*/, const QCameraImageCapture::Error /*_err*/, const QString &_str)
{
	QMessageBox::warning(this, tr("Image Capture Error"), _str);
}

void MainWindow::onCameraStateChanged(QCamera::State /*_state*/)
{
    /*switch (state) {
    case QCamera::ActiveState:
        ui->actionStartCamera->setEnabled(false);
        ui->actionStopCamera->setEnabled(true);
        ui->captureWidget->setEnabled(true);
        ui->actionSettings->setEnabled(true);
        break;
    case QCamera::UnloadedState:
    case QCamera::LoadedState:
        ui->actionStartCamera->setEnabled(true);
        ui->actionStopCamera->setEnabled(false);
        ui->captureWidget->setEnabled(false);
        ui->actionSettings->setEnabled(false);
    }*/
}

void MainWindow::onCameraErrorOccurred(QCamera::Error /*_err*/)
{
	QMessageBox::warning(this, tr("Camera error"), mCamera->errorString());
}

void MainWindow::onReadyForCaptureChanged(bool _ready)
{
	ui->buttonGetShapshot->setEnabled(_ready);
}

