#include "capturablevideosurface.h"
#include <QVideoSurfaceFormat>

CapturableVideoSurface::CapturableVideoSurface(QLabel *_label, QObject *_parent) :
	QAbstractVideoSurface(_parent),
	mLabel(_label),
	mSnapshotQueried(0)
{
}

QList<QVideoFrame::PixelFormat> CapturableVideoSurface::supportedPixelFormats(
	QAbstractVideoBuffer::HandleType _handleType) const
{
	if (_handleType == QAbstractVideoBuffer::NoHandle) {
		return QList<QVideoFrame::PixelFormat>()
				<< QVideoFrame::Format_RGB32;
//				<< QVideoFrame::Format_ARGB32
//				<< QVideoFrame::Format_ARGB32_Premultiplied
//				<< QVideoFrame::Format_RGB565
//				<< QVideoFrame::Format_RGB555;
	} else {
		return QList<QVideoFrame::PixelFormat>();
	}
}

bool CapturableVideoSurface::present(const QVideoFrame &_frame)
{
	QVideoFrame frametodraw(_frame);

	if (!frametodraw.map(QAbstractVideoBuffer::ReadOnly)) {
	   setError(ResourceError);
	   return false;
	}

	QImage image(frametodraw.bits(), frametodraw.width(), frametodraw.height(),
		frametodraw.bytesPerLine(),
		QVideoFrame::imageFormatFromPixelFormat(frametodraw.pixelFormat()));

	image = (surfaceFormat().scanLineDirection() == QVideoSurfaceFormat::BottomToTop) ? image.mirrored()
																					  : image.copy();

	frametodraw.unmap();

	if (mSnapshotQueried > 0) {

		mSnapshotQueried--;
		emit newSnapshot(image);
	}

	mLabel->setPixmap(QPixmap::fromImage(image).scaled(mLabel->width(), mLabel->height(), Qt::KeepAspectRatio));	

	mLabel->repaint();

	return true;
}

void CapturableVideoSurface::querySnapshot()
{
	++mSnapshotQueried;
}


