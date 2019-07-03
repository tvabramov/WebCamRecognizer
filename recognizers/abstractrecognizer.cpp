#include "abstractrecognizer.h"
#include <QSharedPointer>

AbstractRecognizer::AbstractRecognizer(QObject *_parent) :
	QObject(_parent)
{
	qRegisterMetaType<QSharedPointer<Recognition>>("QSharedPointer<Recognition>");
}
