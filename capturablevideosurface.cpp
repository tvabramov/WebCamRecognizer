#include "capturablevideosurface.h"
#include <QVideoSurfaceFormat>
#include <QPainter>

//https://evileg.com/ru/post/81/
CapturableVideoSurface::CapturableVideoSurface(QRectF _boundingRect, QObject *_parent, QGraphicsItem *_parentItem) :
	QAbstractVideoSurface(_parent), QGraphicsItem(_parentItem),
	mBoundingRect(_boundingRect), mFramePainted(false), mSnapshotQueried(false)
{
}

QList<QVideoFrame::PixelFormat> CapturableVideoSurface::supportedPixelFormats(
	QAbstractVideoBuffer::HandleType _handleType) const
{
	if (_handleType == QAbstractVideoBuffer::NoHandle) {
		return QList<QVideoFrame::PixelFormat>()
			<< QVideoFrame::Format_RGB32
			<< QVideoFrame::Format_ARGB32
			<< QVideoFrame::Format_ARGB32_Premultiplied
			<< QVideoFrame::Format_RGB565
			<< QVideoFrame::Format_RGB555;
	} else {
		return QList<QVideoFrame::PixelFormat>();
	}
}

QRectF CapturableVideoSurface::boundingRect() const
{
	return mBoundingRect.isValid() ? mBoundingRect : QRectF(QPointF(0,0), surfaceFormat().sizeHint());
}

void CapturableVideoSurface::paint(QPainter *_painter, const QStyleOptionGraphicsItem */*_option*/, QWidget */*_widget*/)
{
	if (mCurrentFrame.map(QAbstractVideoBuffer::ReadOnly)) {

		QImage image(mCurrentFrame.bits(), mCurrentFrame.width(), mCurrentFrame.height(),
			QVideoFrame::imageFormatFromPixelFormat(mCurrentFrame.pixelFormat()));

		image = (surfaceFormat().scanLineDirection() == QVideoSurfaceFormat::BottomToTop) ? image.mirrored()
																						  : image.copy();

		mCurrentFrame.unmap();

		_painter->drawImage(boundingRect(), image);

		mFramePainted = true;

		if (mSnapshotQueried) {

			mSnapshotQueried = false;
			emit newSnapshot(image);
		}
	} else {

		setError(ResourceError);
	}
}

bool CapturableVideoSurface::present(const QVideoFrame &_frame)
{
	if (!mFramePainted) {
		if (!QAbstractVideoSurface::isActive())
			setError(StoppedError);

		return false;
	} else {
		mCurrentFrame = _frame;
		mFramePainted = false;

		update();

		return true;
	}
}

bool CapturableVideoSurface::start(const QVideoSurfaceFormat &format)
{
	if (isFormatSupported(format)) {

		mFramePainted = true;

		QAbstractVideoSurface::start(format);

		prepareGeometryChange();

		return true;
	} else {
		return false;
	}
}

void CapturableVideoSurface::stop()
{
	mFramePainted = false;

	QAbstractVideoSurface::stop();
}

void CapturableVideoSurface::querySnapshot()
{
	mSnapshotQueried = true;
}
