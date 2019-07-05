#include "testtracker.h"
#include <QDebug>
#include <QVector>

using namespace cv;
using namespace std;

TestTracker::TestTracker(QObject *_parent) :
	AbstractTracker(_parent),
	mTrack(nullptr),
	mCvMultiTracker(MultiTracker::create())
{
}

void TestTracker::track(QSharedPointer<Recognition> _rec)
{
	if (!_rec->isValid()) {

		emit newTrack(QSharedPointer<Track>(new Track(tr("Invalid source data"))));
		return;
	}

//	if (mTrack.items().isEmpty()) {

		// TODO: не обязательно всегда копировать _rec->image()
		QImage image = _rec->image().convertToFormat(QImage::Format_Grayscale8);

		if (image.isNull() || image.format() != QImage::Format_Grayscale8) {

			emit newTrack(QSharedPointer<Track>(new Track(tr("Unsupported image format"))));
			return;
		}

		Mat frame1(image.height(), image.width(), CV_8UC1, const_cast<uchar*>(image.bits()),
			static_cast<size_t>(image.bytesPerLine()));



		//Mat depth_vis; // destination image for visualization
		//normalize(frame1, frame1, 0, 255, CV_MINMAX);
		//depth_vis.convertTo(depth_vis, CV_8UC4);


		//cvtColor(Mat(image.height(), image.width(), CV_8UC4, const_cast<uchar*>(image.bits()),
		//	static_cast<size_t>(image.bytesPerLine())), frame1, cv::COLOR_BGRA2GRAY);

		// https://answers.opencv.org/question/116453/fourier-descriptor-in-opencv/

		Mat frame2 = Mat::zeros(image.height(), image.width(), CV_32FC1);//Mat frame2(image.height(), image.width(), CV_8UC1/*CV_32FC2*/);
		frame1.convertTo(frame2, CV_32FC1, 1.0/255.0);

		QVector<uchar> f1;
		for (auto curr = frame1.data; curr != frame1.dataend; ++curr)
			f1.push_back(*curr);

		QVector<uchar> f2;
		for (auto curr = frame2.data; curr != frame2.dataend; ++curr)
			f2.push_back(*curr);
//source.convertTo(OriginalFloat, CV_32FC1, 1.0/255.0);

qDebug() << f1;
qDebug() << f2;
//qDebug() <<sizeof(frame2.data);
cv::imwrite("D:/out1.bmp", frame1);
cv::imwrite("D:/out2.bmp", frame2);


		vector<Rect> bboxes;
		for (const auto &item : _rec->items())
			bboxes.push_back(Rect(item.rect.left(), item.rect.top(), item.rect.width(), item.rect.height()));

		//Ptr<Tracker> t =TrackerCSRT::create();
		//t->init(frame2, Rect2d(bboxes[0]));
		//for (const auto& bbox : bboxes)
		//	mCvMultiTracker->add(TrackerCSRT::create(), frame2, Rect2d(bbox));

		QList<TrackedItem> trackedItems;
		std::copy(_rec->items().begin(), _rec->items().end(), std::back_inserter(trackedItems));
		mTrack = QSharedPointer<Track>(new Track(image, trackedItems));

		emit newTrack(mTrack);

		// create a video capture object to read videos
//	} else {

//	}
}
