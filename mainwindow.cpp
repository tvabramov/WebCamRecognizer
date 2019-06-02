#include <QCameraInfo>
#include <QMessageBox>
#include <QtWidgets>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>

using namespace cv;
using namespace dnn;

Q_DECLARE_METATYPE(QCameraInfo)

MainWindow::MainWindow(QWidget *_parent) :
    QMainWindow(_parent),
    ui(new Ui::MainWindow),
	mCamera(nullptr),
	mCapturer(nullptr)
{
    ui->setupUi(this);

	// General

	connect(ui->buttonGetShapshot, &QPushButton::clicked, this, &MainWindow::onGetShapshotClicked);

	// Camera selection actions

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

	// Tool bar

	ui->mainToolBar->addAction(ui->actionToggleCamera);
	ui->mainToolBar->addAction(ui->actionToggleLock);

	// For consistency

	onCameraSelected(camerasGroup->checkedAction());
}

MainWindow::~MainWindow()
{
	delete ui;

	delete mCamera;
	delete mCapturer;
}

void MainWindow::setCamera(const QCameraInfo &_cameraInfo)
{
	delete mCamera;
	delete mCapturer;

	mCamera = new QCamera(_cameraInfo);

	connect(mCamera, &QCamera::stateChanged, this, &MainWindow::onCameraStateChanged);
	connect(mCamera, static_cast<void(QCamera::*)(QCamera::Error)>(&QCamera::error),
		this, &MainWindow::onCameraErrorOccurred);
	connect(ui->sliderExposureCompensation, &QSlider::valueChanged, this, &MainWindow::onExposureCompensationSetted);
	connect(mCamera, static_cast<void(QCamera::*)(QCamera::LockStatus, QCamera::LockChangeReason)>(&QCamera::lockStatusChanged),
		this, &MainWindow::onLockStatusChanged);

	mCamera->setViewfinder(ui->viewFinder);

	ui->actionToggleLock->setEnabled(mCamera->supportedLocks() != QCamera::NoLock && mCamera->state() == QCamera::ActiveState);

	onCameraStateChanged(mCamera->state());
	onLockStatusChanged(mCamera->lockStatus(), QCamera::UserRequest);

	// Capturer

	mCapturer = new QCameraImageCapture(mCamera);

	mCapturer->setCaptureDestination(QCameraImageCapture::CaptureToBuffer);
	connect(mCapturer, &QCameraImageCapture::readyForCaptureChanged, this, &MainWindow::onReadyForCaptureChanged);
	connect(mCapturer, &QCameraImageCapture::imageCaptured, this, &MainWindow::onImageCaptured);
	connect(mCapturer, static_cast<void(QCameraImageCapture::*)(int, QCameraImageCapture::Error, const QString &)>(&QCameraImageCapture::error),
		this, &MainWindow::onCapturerErrorOccurred);

	onReadyForCaptureChanged(mCapturer->isReadyForCapture());

	// Other

	mCamera->start();


   //
//    updateRecorderState(mediaRecorder->state());



//    connect(camera, SIGNAL(lockStatusChanged(QCamera::LockStatus,QCamera::LockChangeReason)),
//            this, SLOT(updateLockStatus(QCamera::LockStatus,QCamera::LockChangeReason)));

	// Camera

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

void MainWindow::onCameraSelected(QAction *_action)
{
	setCamera(qvariant_cast<QCameraInfo>(_action->data()));
}

void MainWindow::onCameraToggled(bool _on)
{
	if (_on)
		mCamera->start();
	else
		mCamera->stop();
}

void MainWindow::onLockToggled(bool _on)
{
	if (_on)
		mCamera->searchAndLock();
	else
		mCamera->unlock();
}

void MainWindow::onImageCaptured(int /*_requestId*/, const QImage& _img)
{

	QImage scaledImage = _img.scaled(ui->labelSnapshot->size(),
		Qt::KeepAspectRatio, Qt::SmoothTransformation);

	//ui->labelSnapshot->setPixmap(QPixmap::fromImage(scaledImage));

	QVector<QString> classes = { "background", "aeroplane", "bicycle", "bird", "boat",
		"bottle", "bus", "car", "cat", "chair", "cow", "diningtable",
		"dog", "horse", "motorbike", "person", "pottedplant", "sheep",
		"sofa", "train", "tvmonitor"};

	Net net = dnn::readNetFromCaffe(QString("C:/MobileNetSSD_deploy.prototxt.txt").toStdString(), QString("C:/MobileNetSSD_deploy.caffemodel").toStdString());

	Mat frame = QImageToCvMat(scaledImage, true);


	//https://web-answers.ru/c/opencv-c-hwnd2mat-skrinshot-gt-blobfromimage.html
	Mat blob = dnn::blobFromImage(frame, 0.007843, Size(300, 300), Scalar(127.5, 127.5, 127.5));


	net.setInput(blob);
	Mat detections = net.forward();
	Mat detectionMat(detections.size[2], detections.size[3], CV_32F, detections.ptr<float>());
	for (int i = 0; i < detectionMat.rows; i++) {

		float confidence = detectionMat.at<float>(i, 2);
		if (confidence > 0.8) { //confidenceThreshold
			int idx = static_cast<int>(detectionMat.at<float>(i, 1));
			int xLeftBottom = static_cast<int>(detectionMat.at<float>(i, 3) * frame.cols);
			int yLeftBottom = static_cast<int>(detectionMat.at<float>(i, 4) * frame.rows);
			int xRightTop = static_cast<int>(detectionMat.at<float>(i, 5) * frame.cols);
			int yRightTop = static_cast<int>(detectionMat.at<float>(i, 6) * frame.rows);

			Rect object((int)xLeftBottom, (int)yLeftBottom,
			(int)(xRightTop - xLeftBottom),
			(int)(yRightTop - yLeftBottom));

			rectangle(frame, object, Scalar(0, 255, 0), 2);

			std::string label = QString("%1: %2").arg(classes[idx]).arg(confidence * 100).toStdString();

			putText(frame, label, Point(xLeftBottom + 3, yLeftBottom + 15),
				FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0,255,0));
		}
	}

	//imshow("frame", frame);

	ui->labelSnapshot->setPixmap(QPixmap::fromImage(cvMatToQImage(frame)));
}

// https://asmaloney.com/2013/11/code/converting-between-cvmat-and-qimage-or-qpixmap/
cv::Mat QImageToCvMat(const QImage &inImage, bool inCloneImageData)
{
	  switch ( inImage.format() )
	  {
		 // 8-bit, 4 channel
		 case QImage::Format_ARGB32:
		 case QImage::Format_ARGB32_Premultiplied:
		 {
			cv::Mat  mat( inImage.height(), inImage.width(),
						  CV_8UC4,
						  const_cast<uchar*>(inImage.bits()),
						  static_cast<size_t>(inImage.bytesPerLine())
						  );

			return (inCloneImageData ? mat.clone() : mat);
		 }

		 // 8-bit, 3 channel
		 case QImage::Format_RGB32:
		 {
			if ( !inCloneImageData )
			{
			   qWarning() << "ASM::QImageToCvMat() - Conversion requires cloning so we don't modify the original QImage data";
			}

			cv::Mat  mat( inImage.height(), inImage.width(),
						  CV_8UC4,
						  const_cast<uchar*>(inImage.bits()),
						  static_cast<size_t>(inImage.bytesPerLine())
						  );

			cv::Mat  matNoAlpha;

			cv::cvtColor( mat, matNoAlpha, cv::COLOR_BGRA2BGR );   // drop the all-white alpha channel

			return matNoAlpha;
		 }

		 // 8-bit, 3 channel
		 case QImage::Format_RGB888:
		 {
			if ( !inCloneImageData )
			{
			   qWarning() << "ASM::QImageToCvMat() - Conversion requires cloning so we don't modify the original QImage data";
			}

			QImage   swapped = inImage.rgbSwapped();

			return cv::Mat( swapped.height(), swapped.width(),
							CV_8UC3,
							const_cast<uchar*>(swapped.bits()),
							static_cast<size_t>(swapped.bytesPerLine())
							).clone();
		 }

		 // 8-bit, 1 channel
		 case QImage::Format_Indexed8:
		 {
			cv::Mat  mat( inImage.height(), inImage.width(),
						  CV_8UC1,
						  const_cast<uchar*>(inImage.bits()),
						  static_cast<size_t>(inImage.bytesPerLine())
						  );

			return (inCloneImageData ? mat.clone() : mat);
		 }

		 default:
			qWarning() << "ASM::QImageToCvMat() - QImage format not handled in switch:" << inImage.format();
			break;
	  }

	  return cv::Mat();
}

QImage cvMatToQImage( const cv::Mat &inMat )
{
	  switch ( inMat.type() )
	  {
		 // 8-bit, 4 channel
		 case CV_8UC4:
		 {
			QImage image( inMat.data,
						  inMat.cols, inMat.rows,
						  static_cast<int>(inMat.step),
						  QImage::Format_ARGB32 );

			return image;
		 }

		 // 8-bit, 3 channel
		 case CV_8UC3:
		 {
			QImage image( inMat.data,
						  inMat.cols, inMat.rows,
						  static_cast<int>(inMat.step),
						  QImage::Format_RGB888 );

			return image.rgbSwapped();
		 }

		 // 8-bit, 1 channel
		 case CV_8UC1:
		 {
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
			QImage image( inMat.data,
						  inMat.cols, inMat.rows,
						  static_cast<int>(inMat.step),
						  QImage::Format_Grayscale8 );
#else
			static QVector<QRgb>  sColorTable;

			// only create our color table the first time
			if ( sColorTable.isEmpty() )
			{
			   sColorTable.resize( 256 );

			   for ( int i = 0; i < 256; ++i )
			   {
				  sColorTable[i] = qRgb( i, i, i );
			   }
			}

			QImage image( inMat.data,
						  inMat.cols, inMat.rows,
						  static_cast<int>(inMat.step),
						  QImage::Format_Indexed8 );

			image.setColorTable( sColorTable );
#endif

			return image;
		 }

		 default:
			qWarning() << "ASM::cvMatToQImage() - cv::Mat image type not handled in switch:" << inMat.type();
			break;
	  }

	  return QImage();
}

void MainWindow::onCapturerErrorOccurred(int /*_id*/, const QCameraImageCapture::Error /*_err*/, const QString &_str)
{
	QMessageBox::warning(this, tr("Image Capture Error"), _str);
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
	}
}

void MainWindow::onLockStatusChanged(QCamera::LockStatus _status, QCamera::LockChangeReason /*_reason*/)
{
	switch (_status) {
		case QCamera::Unlocked:
			ui->actionToggleLock->blockSignals(true);
			ui->actionToggleLock->setChecked(false);
			ui->actionToggleLock->setText(tr("Lock Camera"));
			ui->actionToggleLock->setToolTip("Lock Camera");
			ui->actionToggleLock->setStatusTip("Lock Camera");
			ui->actionToggleLock->setWhatsThis("Lock Camera");
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
	QMessageBox::warning(this, tr("Camera error"), mCamera->errorString());
}

void MainWindow::onReadyForCaptureChanged(bool _ready)
{
	ui->buttonGetShapshot->setEnabled(_ready);
}

void MainWindow::onGetShapshotClicked()
{
	mCapturer->capture();
}

void MainWindow::onExposureCompensationSetted(int _index)
{
	mCamera->exposure()->setExposureCompensation(_index * 0.5);
}
