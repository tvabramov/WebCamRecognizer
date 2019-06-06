#include "recognitionitem.h"
#include <QPainter>

RecognitionItem::RecognitionItem(QRectF _boundingRect, int _thres_proc, QObject *_parent, QGraphicsItem *_parentItem) :
	QObject(_parent), QGraphicsItem(_parentItem),
	mBoundingRect(_boundingRect), mConfThres(static_cast<double>(_thres_proc) / 100.0)
{
}

QRectF RecognitionItem::boundingRect() const
{
	return mBoundingRect.isValid() ? mBoundingRect : QRectF(QPointF(0,0), QPointF(mRec.image.width(), mRec.image.height()));
}

void RecognitionItem::paint(QPainter *_painter, const QStyleOptionGraphicsItem */*_option*/, QWidget */*_widget*/)
{
	if (mRec.isEmpty())
		return;

	if (!mRec.isValid()) {

		_painter->setPen(QPen(QColor(255,0,0), 3));
		_painter->setFont(QFont("Times", 10, QFont::Bold));
		_painter->drawText(0, boundingRect().height() / 2, tr("Error: %1").arg(mRec.error));

		return;
	}

	qreal sx = mBoundingRect.width() / static_cast<qreal>(mRec.image.width());
	qreal sy = mBoundingRect.height() / static_cast<qreal>(mRec.image.height());

	_painter->drawImage(boundingRect(), mRec.image);
	_painter->setPen(QPen(QColor(0,255,0), 3));
	_painter->setFont(QFont("Times", 10, QFont::Bold));

	for (const RecognizedItem &item : mRec.items)
		if (item.confidence >= mConfThres && item.type == ITEMCLASSES::CHAIR) {

			// TODO: Сделать более цивильно
			_painter->drawRect(item.rect.topLeft().x() * sx,
							   item.rect.topLeft().y() * sy,
							   item.rect.width() * sx,
							   item.rect.height() * sy);

			_painter->drawText(item.rect.topLeft().x() * sx + 5,
							   item.rect.topLeft().y() * sx + 15,
							   tr("Chair: %1 %").arg(item.confidence * 100.0, 0, 'f', 2));
		}

}

void RecognitionItem::setRecognition(Recognition _rec)
{
	mRec = _rec;

	update();
}

void RecognitionItem::setConfThresholdProc(int _thres)
{
	mConfThres = static_cast<double>(_thres) / 100.0;

	update();
}
