#include "recognizer.h"
#include <QSharedPointer>

Recognizer::Recognizer(QObject *_parent) :
	QObject(_parent)
{
	qRegisterMetaType<QSharedPointer<Recognition>>("QSharedPointer<Recognition>");
}
