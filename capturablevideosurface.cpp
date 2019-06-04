#include "capturablevideosurface.h"
#include <QVideoSurfaceFormat>
#include <QPainter>

//https://evileg.com/ru/post/81/
CapturableVideoSurface::CapturableVideoSurface(QObject *_parent, QGraphicsItem *_parentItem) :
	QAbstractVideoSurface(_parent), QGraphicsItem(_parentItem),
	mSnapshotQueried(0),
	mFramePainted(false)
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
/*
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
}*/
QRectF CapturableVideoSurface::boundingRect() const
{
	return QRectF(QPointF(0,0), surfaceFormat().sizeHint());
}

void CapturableVideoSurface::paint(QPainter *_painter, const QStyleOptionGraphicsItem */*_option*/, QWidget */*_widget*/)
{
	if (mCurrentFrame.map(QAbstractVideoBuffer::ReadOnly)) {

		const QTransform oldTransform = _painter->transform();

		if (surfaceFormat().scanLineDirection() == QVideoSurfaceFormat::BottomToTop) {
		   _painter->scale(1, -1);
		   _painter->translate(0, -boundingRect().height());
		}

		_painter->drawImage(boundingRect(), QImage(
				mCurrentFrame.bits(),
				mCurrentFrame.width(),
				mCurrentFrame.height(),
				QVideoFrame::imageFormatFromPixelFormat(mCurrentFrame.pixelFormat())));

		_painter->setTransform(oldTransform);

		mFramePainted = true;

		mCurrentFrame.unmap();
	}
}

void CapturableVideoSurface::querySnapshot()
{
	++mSnapshotQueried;
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
//		imageFormat = QVideoFrame::imageFormatFromPixelFormat(format.pixelFormat());
		//imageSize = format.frameSize();
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
	//currentFrame = QVideoFrame();
	mFramePainted = false;

	QAbstractVideoSurface::stop();
}
