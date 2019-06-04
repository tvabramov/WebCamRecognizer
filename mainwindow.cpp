#include <QCameraInfo>
#include <QMessageBox>
#include <QtWidgets>
//#include <QGLWidget>
#include "mainwindow.h"
#include "ui_mainwindow.h"

Q_DECLARE_METATYPE(QCameraInfo)

MainWindow::MainWindow(QWidget *_parent) :
    QMainWindow(_parent),
    ui(new Ui::MainWindow),
	mCamera(nullptr),
	mRecognizer(nullptr)
{
    ui->setupUi(this);

	// General
	mSurface = new CapturableVideoSurface;
	QGraphicsScene *scene = new QGraphicsScene(this);
	scene->addItem(mSurface);
	//QGraphicsView *graphicsView = new QGraphicsView(scene);
	ui->graphicsViewFinder->setScene(scene);
#if !defined(QT_NO_OPENGL)
	//ui->graphicsViewFinder->setViewport(new QGLWidget);
#endif


	//mSurface = new CapturableVideoSurface(ui->labelViewFinder, this);
	connect(ui->actionRecognize, &QAction::triggered, mSurface, &CapturableVideoSurface::querySnapshot);
	connect(ui->actionInfiniteRecognition, &QAction::triggered, this, &MainWindow::onInfiniteRecognitionToggled);
	connect(ui->actionAboutProgram, &QAction::triggered, this, &MainWindow::onAboutProgram);
	connect(ui->actionAboutQt, &QAction::triggered, qApp, &QApplication::aboutQt);

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

	// Recognizer

	mRecognizer = new MobileNetSSDRecognizer(nullptr);

	connect(mSurface, &CapturableVideoSurface::newSnapshot, mRecognizer, &MobileNetSSDRecognizer::recognize);
	connect(mRecognizer, &MobileNetSSDRecognizer::newRecognation, this, &MainWindow::onNewRecognation);
	connect(mRecognizer, &MobileNetSSDRecognizer::recognationFailed, this, &MainWindow::onRecognationError);

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
	delete ui;

	delete mCamera;
	delete mRecognizer;
}

void MainWindow::setCamera(const QCameraInfo &_cameraInfo)
{
	delete mCamera;

	mCamera = new QCamera(_cameraInfo);

	connect(mCamera, &QCamera::stateChanged, this, &MainWindow::onCameraStateChanged);
	connect(mCamera, static_cast<void(QCamera::*)(QCamera::Error)>(&QCamera::error),
		this, &MainWindow::onCameraErrorOccurred);
	connect(ui->sliderExposureCompensation, &QSlider::valueChanged, this, &MainWindow::onExposureCompensationSetted);
	connect(mCamera, static_cast<void(QCamera::*)(QCamera::LockStatus, QCamera::LockChangeReason)>(&QCamera::lockStatusChanged),
		this, &MainWindow::onLockStatusChanged);

	mCamera->setViewfinder(mSurface);//(ui->viewFinder);//

	ui->actionToggleLock->setEnabled(mCamera->supportedLocks() != QCamera::NoLock && mCamera->state() == QCamera::ActiveState);
	ui->actionRecognize->setEnabled(!ui->actionInfiniteRecognition->isChecked() && mCamera->state() == QCamera::ActiveState);
	ui->actionInfiniteRecognition->setEnabled(mCamera->state() == QCamera::ActiveState);

	ui->groupBoxCamera->setTitle(tr("Camera [%1]").arg(_cameraInfo.description()));

	// For consistency

	onCameraStateChanged(mCamera->state());
	onLockStatusChanged(mCamera->lockStatus(), QCamera::UserRequest);
	onExposureCompensationSetted(ui->sliderExposureCompensation->value());

	mCamera->start();
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

void MainWindow::onNewRecognation(Recognation _rec)
{
	QPixmap pixmap = QPixmap::fromImage(_rec.image);

	QPainter painter(&pixmap);
	painter.setPen(QPen(QColor(0,255,0), 3));
	painter.setFont(QFont("Times", 10, QFont::Bold));

	double thres = static_cast<double>(ui->sliderConfidenceThreshold->value()) / 100.0;
	for (const RecognizedItem &item : _rec.items)
		if (item.confidence >= thres && item.type == ITEMCLASSES::CHAIR) {
			painter.drawRect(item.rect);
			painter.drawText(item.rect.topLeft().x() + 5, item.rect.topLeft().y() + 15,
				tr("Chair: %1 %").arg(item.confidence * 100.0, 0, 'f', 2));
		}

	ui->labelSnapshot->setMinimumWidth(pixmap.width());
	ui->labelSnapshot->setMaximumWidth(pixmap.width());
	ui->labelSnapshot->setMinimumHeight(pixmap.height());
	ui->labelSnapshot->setMaximumHeight(pixmap.height());
	ui->labelSnapshot->setPixmap(pixmap);

	if (ui->actionInfiniteRecognition->isChecked())
		mSurface->querySnapshot();		//TODO: fix it
}

void MainWindow::onRecognationError(QString _reason)
{
	QMessageBox::warning(this, tr("Recognation Error"), _reason);
}

void MainWindow::onAboutProgram()
{
	QString s = "<font color=red><b><big>" + tr("WebCamRecognizer") + "</big></b></font><br />";
	s += "<font color=green> " + tr("Version: ") + QString(PROGRAMVERSION) + "</font>";

	s += "<hr>" + tr("Application for chairs recognition and counting on webcam snapshots") + "<br /><br />";

	QMessageBox *msgBox = new QMessageBox(this);
	msgBox->setAttribute(Qt::WA_DeleteOnClose);
	msgBox->setWindowTitle(tr("About Program"));
	msgBox->setText(s);
	msgBox->setWindowModality(Qt::NonModal);
	msgBox->setIcon(QMessageBox::Information);
	msgBox->show();
}
