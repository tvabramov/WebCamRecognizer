#include "abstracttracker.h"

AbstractTracker::AbstractTracker(QObject *_parent) :
	QObject(_parent)
{
	qRegisterMetaType<QSharedPointer<Track>>("QSharedPointer<Track>");
}
