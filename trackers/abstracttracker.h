#ifndef ABSTRACTTRACKER_H
#define ABSTRACTTRACKER_H

#include <QObject>
#include <QPointF>
#include <QSharedPointer>
#include "recognizers/abstractrecognizer.h"

struct TrackedItem
{
	RecognizedItem item;
	QList<QPointF> track;

	TrackedItem() {}
	TrackedItem(RecognizedItem _item) :
		item(std::move(_item)) { track.push_back(item.rect.center()); }
};

class Track
{
public:
	Track() {}
	Track(QImage _image, const QList<TrackedItem>& _items) :
		mImage(_image.copy()), mItems(_items) {}
	Track(QString _error) : mError(_error) {}

	const QImage& image() const { return mImage; }
	const QList<TrackedItem>& items() const { return mItems; }
	QString error() const { return mError; }

	bool isValid() const { return !mImage.isNull() && mError.isEmpty(); }

private:
	QImage mImage;
	QList<TrackedItem> mItems;
	QString mError;
};

class AbstractTracker : public QObject
{
	Q_OBJECT

public:
	explicit AbstractTracker(QObject *_parent = nullptr);

signals:
	void newTrack(QSharedPointer<Track> _track);

public slots:
	virtual void track(QSharedPointer<Recognition> _rec) = 0;
};

#endif // ABSTRACTTRACKER_H
