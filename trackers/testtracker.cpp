#include "testtracker.h"
#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>

using namespace cv;
using namespace std;


TestTracker::TestTracker(QObject *_parent) :
	AbstractTracker(_parent)
{
	TrackerCSRT::create();
}

void TestTracker::track(QSharedPointer<Recognition> _rec)
{
	if (!_rec->isValid())
		return;

}
