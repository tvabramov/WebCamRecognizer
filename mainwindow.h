#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCamera>
#include <opencv2/imgproc.hpp>
#include "capturablevideosurface.h"

//enum MatColorOrder {
//	MCO_BGR,
//	MCO_RGB,
//	MCO_BGRA = MCO_BGR,
//	MCO_RGBA = MCO_RGB,
//	MCO_ARGB
//};

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *_parent = nullptr);
    ~MainWindow();

private slots:

	void onCameraSelected(QAction *_action);
	void onCameraToggled(bool _on);
	void onLockToggled(bool _on);
	void onCameraErrorOccurred(QCamera::Error _err);
	void onCameraStateChanged(QCamera::State _state);
	void onLockStatusChanged(QCamera::LockStatus _status, QCamera::LockChangeReason _reason);
	void onImageCaptured(QImage _img);
	void onReadyForCaptureChanged(bool _ready);
	void onExposureCompensationSetted(int _index);

protected:
	void keyPressEvent(QKeyEvent *_event);
	void keyReleaseEvent(QKeyEvent *_event);

private:
    Ui::MainWindow *ui;
	QCamera *mCamera;
	CapturableVideoSurface *mSurface;
	void setCamera(const QCameraInfo &_cameraInfo);
};

cv::Mat QImageToCvMat(const QImage &inImage, bool inCloneImageData = true);
QImage cvMatToQImage( const cv::Mat &inMat );

#endif // MAINWINDOW_H
