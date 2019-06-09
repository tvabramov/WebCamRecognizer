#ifndef RECOGNITIONITEM_H
#define RECOGNITIONITEM_H

#include <QObject>
#include <QGraphicsItem>

class Recognition;

class RecognitionItem : public QObject, public QGraphicsItem
{
	Q_OBJECT
	Q_INTERFACES(QGraphicsItem)

public:
	explicit RecognitionItem(QRectF _boundingRect, int _thres_proc, QObject *_parent = nullptr, QGraphicsItem *_parentItem = nullptr);

	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr);

public slots:
	void setRecognition(QSharedPointer<Recognition> _rec);
	void setConfThresholdProc(int _thres);

private:
	QRectF mBoundingRect;
	QSharedPointer<Recognition> mRec;
	double mConfThres;
};

#endif // RECOGNITIONITEM_H
