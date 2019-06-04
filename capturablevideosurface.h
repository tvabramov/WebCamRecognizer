#ifndef CAPTURABLEVIDEOSURFACE_H
#define CAPTURABLEVIDEOSURFACE_H

#include <QAbstractVideoSurface>
#include <QGraphicsItem>

class CapturableVideoSurface : public QAbstractVideoSurface, public QGraphicsItem
{
	Q_OBJECT
	Q_INTERFACES(QGraphicsItem)

public:
	explicit CapturableVideoSurface(QRectF _boundingRect, QObject *_parent = nullptr, QGraphicsItem *_parentItem = nullptr);

	QList<QVideoFrame::PixelFormat> supportedPixelFormats(
			QAbstractVideoBuffer::HandleType _handleType = QAbstractVideoBuffer::NoHandle) const;

	bool present(const QVideoFrame &_frame);
	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
	bool start(const QVideoSurfaceFormat &format);
	void stop();

signals:
	void newSnapshot(QImage _img);

private:
	QRectF mBoundingRect;
	QVideoFrame mCurrentFrame;
	bool mFramePainted;
};

#endif // CAPTURABLEVIDEOSURFACE_H
