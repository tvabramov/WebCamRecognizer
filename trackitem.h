#ifndef TRACKITEM_H
#define TRACKITEM_H

#include <QObject>
#include <QGraphicsItem>

class Track;

class TrackItem : public QObject, public QGraphicsItem
{
	Q_OBJECT
	Q_INTERFACES(QGraphicsItem)

public:
	explicit TrackItem(QRectF _boundingRect, int _thres_proc, QObject *_parent = nullptr, QGraphicsItem *_parentItem = nullptr);

	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr);

public slots:
	void setTrack(QSharedPointer<Track> _track);
	void setConfThresholdProc(int _thres);

private:
	QRectF mBoundingRect;
	QSharedPointer<Track> mTrack;
	double mConfThres;
};


#endif // TRACKITEM_H
