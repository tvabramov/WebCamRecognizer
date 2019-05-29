#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCamera>
#include <QCameraImageCapture>

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

	void onCapturerErrorOccurred(int _id, QCameraImageCapture::Error _err, const QString &_str);
	void onCameraErrorOccurred(QCamera::Error _err);
	void onCameraStateChanged(QCamera::State _state);
	void onImageCaptured(int _requestId, const QImage &_img);
	void onReadyForCaptureChanged(bool _ready);

protected:
	void keyPressEvent(QKeyEvent *_event);
	void keyReleaseEvent(QKeyEvent *_event);

private:
    Ui::MainWindow *ui;

	QCamera *mCamera;
	QCameraImageCapture *mCapturer;
};

#endif // MAINWINDOW_H
