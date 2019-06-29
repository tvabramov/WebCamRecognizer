#ifndef TESTTRACKER_H
#define TESTTRACKER_H

#include "tracker.h"

class TestTracker : public Tracker
{
	Q_OBJECT

public:
	explicit TestTracker(QObject *_parent = nullptr);

public slots:
	virtual void track(QSharedPointer<Recognition> _rec) override;

protected:
	Track mTrack;
};

#endif // TESTTRACKER_H
