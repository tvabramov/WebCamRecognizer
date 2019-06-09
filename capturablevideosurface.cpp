#include "capturablevideosurface.h"
#include <QVideoSurfaceFormat>
#include <QPainter>

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
	return mBoundingRect.isValid() ? mBoundingRect : QRectF(QPointF(0, 0), surfaceFormat().sizeHint());
}

/**
 * @brief   CapturableVideoSurface::paint
 * @details Выполняем функцию прорисовки и (при необходимости)
 *          отправки сигнала с image.
 * @note    1. Не очень хорошая идея совещать прорисовку и отправку
 *             сигнала, т.к. это разные вещи.
 *          2. Антипаттерн: Используются разные алгоритмы для
 *             получения image когда нужно отправлять сигнал и когда нет.
 *             Зато это быстрее.
 */
void CapturableVideoSurface::paint(QPainter *_painter, const QStyleOptionGraphicsItem */*_option*/, QWidget */*_widget*/)
{
	if (mCurrentFrame.map(QAbstractVideoBuffer::ReadOnly)) {

		if (mSnapshotQueried) {

			QImage image = QImage(mCurrentFrame.bits(), mCurrentFrame.width(), mCurrentFrame.height(),
				QVideoFrame::imageFormatFromPixelFormat(mCurrentFrame.pixelFormat())).copy();

			mCurrentFrame.unmap();

			if (surfaceFormat().scanLineDirection() == QVideoSurfaceFormat::BottomToTop)
				image = image.mirrored();

			_painter->drawImage(boundingRect(), image);

			mFramePainted = true;
			mSnapshotQueried = false;

			emit newSnapshot(image);
		} else {

			const QTransform oldTransform = _painter->transform();

			if (surfaceFormat().scanLineDirection() == QVideoSurfaceFormat::BottomToTop) {
				_painter->scale(1, -1);
				_painter->translate(0, -boundingRect().height());
			}

			_painter->drawImage(boundingRect(), QImage(mCurrentFrame.bits(), mCurrentFrame.width(),
				mCurrentFrame.height(), QVideoFrame::imageFormatFromPixelFormat(mCurrentFrame.pixelFormat())));

			_painter->setTransform(oldTransform);

			mFramePainted = true;

			mCurrentFrame.unmap();
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

bool CapturableVideoSurface::start(const QVideoSurfaceFormat &_format)
{
	if (isFormatSupported(_format)) {

		mFramePainted = true;

		QAbstractVideoSurface::start(_format);

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
