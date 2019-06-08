#ifndef MOBILENETSSDRECOGNIZER_H
#define MOBILENETSSDRECOGNIZER_H

#include <QObject>
#include <QRect>
#include <QImage>
#include <QMutex>
#include <opencv2/dnn.hpp>
#include <chrono>

enum class ITEMCLASSES
{
	BACKGROUND, AEROPLANE, BICYCLE, BIRD, BOAT,
	BOTTLE, BUS, CAR, CAT, CHAIR, COW, DININGTABLE,
	DOG, HORSE, MOTORBIKE, PERSON, POTTEDPLANT, SHEEP,
	SOFR, TRAIN, TVMONITOR, INVALID, COUNT
};

struct RecognizedItem {

	ITEMCLASSES type;
	double confidence;
	QRect rect;

	RecognizedItem() :
		type(ITEMCLASSES::INVALID), confidence(0.0) {}
	RecognizedItem(ITEMCLASSES _type, double _confidence, QRect _rect) :
		type(_type), confidence(_confidence), rect(_rect) {}
};

struct Recognition {

	QImage image;
	QVector<RecognizedItem> items;
	QString error;
	int duration_ms;

	Recognition() {}
	Recognition(QImage _image) :
		image(_image.copy()) {}
	Recognition(QImage _image, QVector<RecognizedItem> _items) :
		image(_image.copy()), items(_items) {}
	Recognition(QString _error) : error(_error) {}

	bool isValid() { return !image.isNull() && error.isEmpty(); }
	bool isEmpty() { return image.isNull() && error.isEmpty(); }
};

class MobileNetSSDRecognizer : public QObject
{
	Q_OBJECT

public:
	explicit MobileNetSSDRecognizer(QObject *_parent = nullptr);

signals:
	void newRecognition(Recognition);

public slots:
	void recognize(QImage _image);

private:
	cv::dnn::Net mNet;
};

#endif // MOBILENETSSDRECOGNIZER_H
