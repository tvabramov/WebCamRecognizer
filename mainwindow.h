#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCamera>

namespace Ui {
class MainWindow;
}

class QThread;
class CapturableVideoSurface;
class MobileNetSSDRecognizer;
class RecognitionItem;
class Recognition;

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
	void onInfiniteRecognitionToggled(bool _on);
	void onCameraErrorOccurred(QCamera::Error _err);
	void onCameraStateChanged(QCamera::State _state);
	void onLockStatusChanged(QCamera::LockStatus _status, QCamera::LockChangeReason _reason);
	void onExposureCompensationSetted(int _index);
	void onNewRecognition();
	void onAboutProgram();

protected:
	void keyPressEvent(QKeyEvent *_event);
	void keyReleaseEvent(QKeyEvent *_event);

private:
    Ui::MainWindow *ui;
	QCamera *mCamera;
	CapturableVideoSurface *mSurface;
	MobileNetSSDRecognizer *mRecognizer;
	QThread *mRecognizerThread;
	RecognitionItem *mRecItem;

	void setCamera(const QCameraInfo &_cameraInfo);
};

#endif // MAINWINDOW_H
