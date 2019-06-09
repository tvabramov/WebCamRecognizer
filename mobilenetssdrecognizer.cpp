#include <QFile>
#include <QSharedPointer>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include "mobilenetssdrecognizer.h"

using namespace cv;
using namespace dnn;
using namespace std;

MobileNetSSDRecognizer::MobileNetSSDRecognizer(QObject *_parent) :
	QObject(_parent)
{
	qRegisterMetaType<QSharedPointer<Recognition>>("QSharedPointer<Recognition>");

	QFile fileProto(":/model/MobileNetSSD_deploy.prototxt.txt");
	fileProto.open(QIODevice::ReadOnly);
	QByteArray bufferProto = fileProto.readAll();

	QFile fileModel(":/model/MobileNetSSD_deploy.caffemodel");
	fileModel.open(QIODevice::ReadOnly);
	QByteArray bufferModel = fileModel.readAll();

	mNet = readNetFromCaffe(bufferProto.data(), bufferProto.size(), bufferModel.data(), bufferModel.size());
}

void MobileNetSSDRecognizer::recognize(QImage _image)
{
	auto time_begin = chrono::high_resolution_clock::now();

	if (_image.isNull()) {

		emit newRecognition(QSharedPointer<Recognition>(new Recognition(tr("Null image"))));
		return;
	}

	if (_image.format() != QImage::Format_RGB32)
		_image = _image.convertToFormat(QImage::Format_RGB32);

	if (_image.isNull() || _image.format() != QImage::Format_RGB32) {

		emit newRecognition(QSharedPointer<Recognition>(new Recognition(tr("Unsupported image format"))));
		return;
	}

	Mat frame;
	cvtColor(Mat(_image.height(), _image.width(), CV_8UC4, const_cast<uchar*>(_image.bits()),
		static_cast<size_t>(_image.bytesPerLine())), frame, cv::COLOR_BGRA2BGR);

	//https://web-answers.ru/c/opencv-c-hwnd2mat-skrinshot-gt-blobfromimage.html
	Mat blob = dnn::blobFromImage(frame, 0.007843, Size(300, 300), Scalar(127.5, 127.5, 127.5));

	mNet.setInput(blob);
	Mat detections = mNet.forward();
	Mat detectionMat(detections.size[2], detections.size[3], CV_32F, detections.ptr<float>());
	QList<RecognizedItem> items;
	for (int i = 0; i < detectionMat.rows; i++) {

		float confidence = detectionMat.at<float>(i, 2);

		int idx = static_cast<int>(detectionMat.at<float>(i, 1));
		int xLeftBottom = static_cast<int>(detectionMat.at<float>(i, 3) * frame.cols);
		int yLeftBottom = static_cast<int>(detectionMat.at<float>(i, 4) * frame.rows);
		int xRightTop = static_cast<int>(detectionMat.at<float>(i, 5) * frame.cols);
		int yRightTop = static_cast<int>(detectionMat.at<float>(i, 6) * frame.rows);

		QRect rect(xLeftBottom, yLeftBottom, xRightTop - xLeftBottom, yRightTop - yLeftBottom);

		items.push_back(RecognizedItem(static_cast<ITEMCLASSES>(idx), confidence, rect));
	}

	emit newRecognition(QSharedPointer<Recognition>(new Recognition(_image, items,
		chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - time_begin).count())));
}

