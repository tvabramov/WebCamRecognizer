#include <QPainter>
#include "recognitionitem.h"
#include "recognizers/abstractrecognizer.h"

RecognitionItem::RecognitionItem(QRectF _boundingRect, int _thres_proc, QObject *_parent, QGraphicsItem *_parentItem) :
	QObject(_parent), QGraphicsItem(_parentItem),
	mBoundingRect(_boundingRect), mConfThres(static_cast<double>(_thres_proc) / 100.0)
{
}

QRectF RecognitionItem::boundingRect() const
{
	if (mBoundingRect.isValid())
		return mBoundingRect;
	else if (mRec && !mRec->image().isNull())
		return QRectF(QPointF(0, 0), QPointF(mRec->image().width(), mRec->image().height()));
	else
		return QRectF(QPointF(0, 0), QPointF(-1.0, -1.0));
}

void RecognitionItem::paint(QPainter *_painter, const QStyleOptionGraphicsItem */*_option*/, QWidget */*_widget*/)
{
	if (!mRec)
		return;

	if (!mRec->isValid()) {

		_painter->setPen(QPen(QColor(255,0,0), 3));
		_painter->setFont(QFont("Times", 10, QFont::Bold));
		_painter->drawText(0, boundingRect().height() / 2, tr("Error: %1").arg(mRec->error()));

		return;
	}

	qreal sx = boundingRect().width() / static_cast<qreal>(mRec->image().width());
	qreal sy = boundingRect().height() / static_cast<qreal>(mRec->image().height());

	_painter->drawImage(boundingRect(), mRec->image());
	_painter->setPen(QPen(QColor(0,255,0), 3));
	_painter->setFont(QFont("Times", 10, QFont::Bold));

	int chairs_count = 0;
	for (const RecognizedItem &item : mRec->items())
		if (item.confidence >= mConfThres && item.type == ITEMCLASSES::PERSON) {

			// TODO: Сделать более цивильно
			_painter->drawRect(item.rect.topLeft().x() * sx,
							   item.rect.topLeft().y() * sy,
							   item.rect.width() * sx,
							   item.rect.height() * sy);

			_painter->drawText(item.rect.topLeft().x() * sx + 5,
							   item.rect.topLeft().y() * sx + 15,
							   tr("Person: %1 %").arg(item.confidence * 100.0, 0, 'f', 2));

			++chairs_count;
		}

	_painter->setPen(QPen(QColor(0,255,255), 3));
	_painter->drawText(boundingRect().bottomLeft().x() + 5, boundingRect().bottomLeft().y() - 10,
		tr("Chairs Count = %1 [Duration = %2 ms]").arg(chairs_count).arg(mRec->durationMs()));
}

void RecognitionItem::setRecognition(QSharedPointer<Recognition> _rec)
{
	mRec = _rec;

	update();
}

void RecognitionItem::setConfThresholdProc(int _thres)
{
	mConfThres = static_cast<double>(_thres) / 100.0;

	update();
}
