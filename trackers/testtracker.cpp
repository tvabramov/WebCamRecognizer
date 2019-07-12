#include "testtracker.h"
#include <QDebug>
#include <QVector>

using namespace cv;
using namespace std;

TestTracker::TestTracker(QObject *_parent) :
	AbstractTracker(_parent),
	mTrack(nullptr),
//	mCvTracker(TrackerCSRT::create()),
	mCvMultiTracker(cv::MultiTracker::create())
{
}

void TestTracker::track(QSharedPointer<Recognition> _rec)
{
	if (!_rec->isValid()) {

		emit newTrack(QSharedPointer<Track>(new Track(tr("Invalid source data"))));
		return;
	}

	// TODO: не обязательно всегда копировать _rec->image()
	QImage image = _rec->image().convertToFormat(QImage::Format_RGB32);

	if (image.isNull() || image.format() != QImage::Format_RGB32) {

		emit newTrack(QSharedPointer<Track>(new Track(tr("Unsupported image format"))));
		return;
	}

	Mat frame1;

	cvtColor(Mat(image.height(), image.width(), CV_8UC4, const_cast<uchar*>(image.bits()),
		static_cast<size_t>(image.bytesPerLine())), frame1, cv::COLOR_BGRA2BGR);

	if (!mTrack) {

		QList<RecognizedItem> recItems;
		std::copy_if(_rec->items().begin(), _rec->items().end(), back_inserter(recItems),
			[](const auto &el){ return el.confidence > 0.9 && el.type == ITEMCLASSES::PERSON; });

		if (recItems.isEmpty()) {

			emit newTrack(QSharedPointer<Track>(new Track(tr("There is no objects to track"))));
			return;
		}

		for (const auto & el: recItems)
			if (!mCvMultiTracker->add(TrackerCSRT::create(), frame1,
				Rect2d(el.rect.left(), el.rect.top(), el.rect.width(), el.rect.height()))) {

				emit newTrack(QSharedPointer<Track>(new Track(tr("Can not init CV Tracker"))));
				return;
			}

		QList<TrackedItem> trackedItems;
		std::copy(recItems.begin(), recItems.end(), back_inserter(trackedItems));

		mTrack = QSharedPointer<Track>(new Track(image, trackedItems));

		emit newTrack(mTrack);

	} else {

		assert(static_cast<int>(mCvMultiTracker->getObjects().size()) == mTrack->items().size());

		if (!mCvMultiTracker->update(frame1)) {

			emit newTrack(QSharedPointer<Track>(new Track(tr("Can not relocate the object"))));
			return;
		}

		auto trackedItems = mTrack->items();

		for (int i = 0; i < trackedItems.size(); ++i) {

			auto rd = mCvMultiTracker->getObjects()[i];

			trackedItems[i].item.rect.setRect(rd.x, rd.y, rd.width, rd.height);
			trackedItems[i].track.push_back(QPointF(rd.x + rd.width / 2.0, rd.y + rd.height / 2.0));
		}

		mTrack = QSharedPointer<Track>(new Track(image, trackedItems));

		emit newTrack(mTrack);
	}
}
