#include "testtracker.h"
#include <QDebug>
#include <QVector>

using namespace cv;
using namespace std;

TestTracker::TestTracker(QObject *_parent) :
	AbstractTracker(_parent),
	mTrack(nullptr),
	mCvTracker(TrackerCSRT::create())
{
}

void TestTracker::track(QSharedPointer<Recognition> _rec)
{
	if (!_rec->isValid()) {

		emit newTrack(QSharedPointer<Track>(new Track(tr("Invalid source data"))));
		return;
	}

	auto main_item = std::find_if(_rec->items().begin(), _rec->items().end(),
		[](const auto &el){ return el.confidence > 0.9 && el.type == ITEMCLASSES::PERSON; });

	if (main_item == _rec->items().end()) {

		emit newTrack(QSharedPointer<Track>(new Track(tr("None objects to track"))));
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

		if (!mCvTracker->init(frame1, Rect2d(main_item->rect.left(), main_item->rect.top(),
			main_item->rect.width(), main_item->rect.height()))) {

			emit newTrack(QSharedPointer<Track>(new Track(tr("Can not init CV Tracker"))));
			return;
		}

		//Rect2d rd;
		//t->update(frame1, rd);
//		for (const auto& bbox : bboxes)
//			mCvMultiTracker->add(TrackerCSRT::create(), frame1, Rect2d(bbox));

		QList<TrackedItem> trackedItems;
		trackedItems.push_back(TrackedItem(RecognizedItem(main_item->type, main_item->confidence, main_item->rect)));		// Recognized
		trackedItems.push_back(TrackedItem(RecognizedItem(main_item->type, main_item->confidence, main_item->rect)));		// Tracked
		//std::copy(_rec->items().begin(), _rec->items().end(), std::back_inserter(trackedItems));
		mTrack = QSharedPointer<Track>(new Track(image, trackedItems));

		emit newTrack(mTrack);

	} else {

		assert(mTrack->items().size() == 2);

		Rect2d rd;
		if (!mCvTracker->update(frame1, rd)) {

			emit newTrack(QSharedPointer<Track>(new Track(tr("Can not relocate the object"))));
			return;
		}

		auto trackedItems = mTrack->items();

		trackedItems.front() = TrackedItem(*main_item);

		trackedItems.back().item.rect.setRect(rd.x, rd.y, rd.width, rd.height);
		trackedItems.back().track.push_back(QPointF(rd.x + rd.width / 2.0, rd.y + rd.height / 2.0));

		mTrack = QSharedPointer<Track>(new Track(image, trackedItems));
//qDebug() << rd.x + rd.width / 2 << " " << rd.y + rd.height / 2;

		emit newTrack(mTrack);
	}
}
