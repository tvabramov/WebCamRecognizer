#ifndef TESTTRACKER_H
#define TESTTRACKER_H

#include "abstracttracker.h"
#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>

class TestTracker : public AbstractTracker
{
	Q_OBJECT

public:
	explicit TestTracker(QObject *_parent = nullptr);

public slots:
	virtual void track(QSharedPointer<Recognition> _rec) override;

protected:
	QSharedPointer<Track> mTrack;
	//cv::Ptr<cv::MultiTracker> mCvMultiTracker;
//	cv::Ptr<cv::Tracker> mCvTracker;

	cv::Ptr<cv::MultiTracker> mCvMultiTracker;
};

#endif // TESTTRACKER_H
