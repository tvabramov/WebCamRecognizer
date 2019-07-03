#ifndef TESTTRACKER_H
#define TESTTRACKER_H

#include "abstracttracker.h"

class TestTracker : public AbstractTracker
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
