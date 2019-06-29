#ifndef MOBILENETSSDRECOGNIZER_H
#define MOBILENETSSDRECOGNIZER_H

#include <QObject>
#include <opencv2/dnn.hpp>
#include "recognizer.h"

class MobileNetSSDRecognizer : public Recognizer
{
	Q_OBJECT

public:
	explicit MobileNetSSDRecognizer(QObject *_parent = nullptr);

public slots:
	virtual void recognize(QImage _image) override;

private:
	cv::dnn::Net mNet;
};

#endif // MOBILENETSSDRECOGNIZER_H
