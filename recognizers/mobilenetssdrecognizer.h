#ifndef MOBILENETSSDRECOGNIZER_H
#define MOBILENETSSDRECOGNIZER_H

#include <QObject>
#include <opencv2/dnn.hpp>
#include "abstractrecognizer.h"

class MobileNetSSDRecognizer : public AbstractRecognizer
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
