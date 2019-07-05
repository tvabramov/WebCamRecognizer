#include <QPainter>
#include "trackitem.h"
#include "trackers/abstracttracker.h"

TrackItem::TrackItem(QRectF _boundingRect, int _thres_proc, QObject *_parent, QGraphicsItem *_parentItem) :
	QObject(_parent), QGraphicsItem(_parentItem),
	mBoundingRect(_boundingRect), mConfThres(static_cast<double>(_thres_proc) / 100.0)
{
}

QRectF TrackItem::boundingRect() const
{
	if (mBoundingRect.isValid())
		return mBoundingRect;
	else if (mTrack && !mTrack->image().isNull())
		return QRectF(QPointF(0, 0), QPointF(mTrack->image().width(), mTrack->image().height()));
	else
		return QRectF(QPointF(0, 0), QPointF(-1.0, -1.0));
}

void TrackItem::paint(QPainter *_painter, const QStyleOptionGraphicsItem */*_option*/, QWidget */*_widget*/)
{
	if (!mTrack)
		return;

	if (!mTrack->isValid()) {

		_painter->setPen(QPen(QColor(255,0,0), 3));
		_painter->setFont(QFont("Times", 10, QFont::Bold));
		_painter->drawText(0, boundingRect().height() / 2, tr("Error: %1").arg(mTrack->error()));

		return;
	}

	qreal sx = boundingRect().width() / static_cast<qreal>(mTrack->image().width());
	qreal sy = boundingRect().height() / static_cast<qreal>(mTrack->image().height());

	_painter->drawImage(boundingRect(), mTrack->image());
	_painter->setPen(QPen(QColor(0,255,0), 3));
	_painter->setFont(QFont("Times", 10, QFont::Bold));

	for (const TrackedItem &item : mTrack->items())
		if (item.item.confidence >= mConfThres && item.item.type == ITEMCLASSES::PERSON) {

			// TODO: Сделать более цивильно
			_painter->drawRect(item.item.rect.topLeft().x() * sx,
							   item.item.rect.topLeft().y() * sy,
							   item.item.rect.width() * sx,
							   item.item.rect.height() * sy);

			_painter->drawText(item.item.rect.topLeft().x() * sx + 5,
							   item.item.rect.topLeft().y() * sx + 15,
							   tr("Person: %1 %").arg(item.item.confidence * 100.0, 0, 'f', 2));
		}

	//_painter->setPen(QPen(QColor(0,255,255), 3));
	//_painter->drawText(boundingRect().bottomLeft().x() + 5, boundingRect().bottomLeft().y() - 10,
	//	tr("Chairs Count = %1 [Duration = %2 ms]").arg(chairs_count).arg(mTrack->durationMs()));
}

void TrackItem::setTrack(QSharedPointer<Track> _track)
{
	mTrack = _track;

	update();
}

void TrackItem::setConfThresholdProc(int _thres)
{
	mConfThres = static_cast<double>(_thres) / 100.0;

	update();
}
