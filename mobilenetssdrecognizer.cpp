#include <QFile>
#include <QRect>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include "mobilenetssdrecognizer.h"

using namespace cv;
using namespace dnn;
using namespace std;

MobileNetSSDRecognizer::MobileNetSSDRecognizer(QObject *_parent) :
	QObject(_parent)
{
	QFile fileProto(":/model/MobileNetSSD_deploy.prototxt.txt");
	fileProto.open(QIODevice::ReadOnly);
	QByteArray bufferProto = fileProto.readAll();

	QFile fileModel(":/model/MobileNetSSD_deploy.caffemodel");
	fileModel.open(QIODevice::ReadOnly);
	QByteArray bufferModel = fileModel.readAll();

	vector<uchar> bufferProto2, bufferModel2;

	copy(bufferProto.begin(), bufferProto.end(), back_inserter(bufferProto2));
	copy(bufferModel.begin(), bufferModel.end(), back_inserter(bufferModel2));

	mNet = readNetFromCaffe(bufferProto2, bufferModel2);
}

void MobileNetSSDRecognizer::recognize(QImage _image)
{
	if (_image.format() != QImage::Format_RGB32)
		emit recognationFailed(tr("Incorrect image format"));

	// Ограничиваем размер
	_image = _image.scaled(QSize(640, 480),
		Qt::KeepAspectRatio, Qt::SmoothTransformation);

	Mat frame;
	cvtColor(Mat(_image.height(), _image.width(), CV_8UC4, const_cast<uchar*>(_image.bits()),
		static_cast<size_t>(_image.bytesPerLine())), frame, cv::COLOR_BGRA2BGR);


	//https://web-answers.ru/c/opencv-c-hwnd2mat-skrinshot-gt-blobfromimage.html
	Mat blob = dnn::blobFromImage(frame, 0.007843, Size(300, 300), Scalar(127.5, 127.5, 127.5));

	mNet.setInput(blob);
	Mat detections = mNet.forward();
	Mat detectionMat(detections.size[2], detections.size[3], CV_32F, detections.ptr<float>());
	Recognation rec;
	for (int i = 0; i < detectionMat.rows; i++) {

		float confidence = detectionMat.at<float>(i, 2);

		int idx = static_cast<int>(detectionMat.at<float>(i, 1));
		int xLeftBottom = static_cast<int>(detectionMat.at<float>(i, 3) * frame.cols);
		int yLeftBottom = static_cast<int>(detectionMat.at<float>(i, 4) * frame.rows);
		int xRightTop = static_cast<int>(detectionMat.at<float>(i, 5) * frame.cols);
		int yRightTop = static_cast<int>(detectionMat.at<float>(i, 6) * frame.rows);

		QRect rect((int)xLeftBottom, (int)yLeftBottom,
		(int)(xRightTop - xLeftBottom),
		(int)(yRightTop - yLeftBottom));

		rec.items.push_back(RecognizedItem(static_cast<ITEMCLASSES>(idx), confidence, rect));
	}

	rec.image = _image;

	emit newRecognation(rec);
}

