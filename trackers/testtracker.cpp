#include "testtracker.h"

TestTracker::TestTracker(QObject *_parent) :
	Tracker(_parent)
{

}

void TestTracker::track(QSharedPointer<Recognition> _rec)
{
	if (!_rec->isValid())
		return;

}
