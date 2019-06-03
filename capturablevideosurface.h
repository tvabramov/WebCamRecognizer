#ifndef CAPTURABLEVIDEOSURFACE_H
#define CAPTURABLEVIDEOSURFACE_H

#include <QAbstractVideoSurface>
#include <QLabel>

class CapturableVideoSurface : public QAbstractVideoSurface
{
	Q_OBJECT

public:
	explicit CapturableVideoSurface(QLabel *mLabel, QObject *_parent = nullptr);

	QList<QVideoFrame::PixelFormat> supportedPixelFormats(
			QAbstractVideoBuffer::HandleType _handleType = QAbstractVideoBuffer::NoHandle) const;

	bool present(const QVideoFrame &_frame);

signals:
	void newSnapshot(QImage _img);

public slots:
	void querySnapshot();

private:
	QLabel *mLabel;
	int mSnapshotQueried;
};

#endif // CAPTURABLEVIDEOSURFACE_H
