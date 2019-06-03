#ifndef MOBILENETSSDRECOGNIZER_H
#define MOBILENETSSDRECOGNIZER_H

#include <QObject>
#include <QRect>
#include <QImage>
#include <QMutex>
#include <opencv2/dnn.hpp>

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

struct Recognation {

	QImage image;
	QVector<RecognizedItem> items;

	Recognation() {}
	Recognation(QImage _image) :
		image(_image.copy()) {}
	Recognation(QImage _image, QVector<RecognizedItem> _items) :
		image(_image.copy()), items(_items) {}
};

class MobileNetSSDRecognizer : public QObject
{
	Q_OBJECT

public:
	explicit MobileNetSSDRecognizer(QObject *_parent = nullptr);

signals:
	void newRecognation(Recognation);
	void recognationFailed(QString reason);

public slots:
	void recognize(QImage _image);

private:
	cv::dnn::Net mNet;
};

#endif // MOBILENETSSDRECOGNIZER_H
