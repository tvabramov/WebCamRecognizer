#ifndef ABSTRACTRECOGNIZER_H
#define ABSTRACTRECOGNIZER_H

#include <QObject>
#include <QRect>
#include <QImage>

enum class ITEMCLASSES
{
	BACKGROUND, AEROPLANE, BICYCLE, BIRD, BOAT,
	BOTTLE, BUS, CAR, CAT, CHAIR, COW, DININGTABLE,
	DOG, HORSE, MOTORBIKE, PERSON, POTTEDPLANT, SHEEP,
	SOFR, TRAIN, TVMONITOR, INVALID, COUNT
};

struct RecognizedItem
{
	ITEMCLASSES type;
	double confidence;
	QRect rect;

	RecognizedItem() :
		type(ITEMCLASSES::INVALID), confidence(0.0) {}
	RecognizedItem(ITEMCLASSES _type, double _confidence, QRect _rect) :
		type(_type), confidence(_confidence), rect(_rect) {}
};

class Recognition
{
public:
	Recognition() : mDurationMs(0) {}
	Recognition(QImage _image, const QList<RecognizedItem>& _items, int _duration_ms) :
		mImage(_image.copy()), mItems(_items), mDurationMs(_duration_ms) {}
	Recognition(QString _error) : mError(_error), mDurationMs(0) {}

	const QImage& image() const { return mImage; }
	const QList<RecognizedItem>& items() const { return mItems; }
	QString error() const { return mError; }
	int durationMs() const { return mDurationMs; }

	bool isValid() const { return !mImage.isNull() && mError.isEmpty(); }

private:
	QImage mImage;
	QList<RecognizedItem> mItems;
	QString mError;
	int mDurationMs;
};

class AbstractRecognizer : public QObject
{
	Q_OBJECT

public:
	explicit AbstractRecognizer(QObject *_parent = nullptr);

signals:
	void newRecognition(QSharedPointer<Recognition> _rec);

public slots:
	virtual void recognize(QImage _image) = 0;
};

#endif // ABSTRACTRECOGNIZER_H
