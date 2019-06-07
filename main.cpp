#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	// Общие настройки
	QTranslator translator;
	translator.load("WebCamRecognizer_ru.qm");
	a.installTranslator(&translator);

	QCoreApplication::setApplicationName(QObject::tr("WebCamRecognizer"));
	QCoreApplication::setOrganizationName(QObject::tr("Timofey Abramov"));
	QCoreApplication::setApplicationVersion(PROGRAMVERSION);

    MainWindow w;
    w.show();

    return a.exec();
}
