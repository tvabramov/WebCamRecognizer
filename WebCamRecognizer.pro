#-------------------------------------------------
#
# Project created by QtCreator 2019-05-29T14:20:39
#
#-------------------------------------------------

QT       += core gui multimedia multimediawidgets opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = WebCamRecognizer
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += resources_big c++14

QMAKE_CXXFLAGS += -Wall

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += $$(OPENCV_SDK_DIR)/include

LIBS += -L$$(OPENCV_SDK_DIR)/x86/mingw/lib \
        -lopencv_core410        \
        -lopencv_dnn410         \
        -lopencv_imgproc410

SOURCES += main.cpp\
        mainwindow.cpp \
    capturablevideosurface.cpp \
    recognizers/mobilenetssdrecognizer.cpp \
    recognitionitem.cpp \
    recognizers/recognizer.cpp

HEADERS  += mainwindow.h \
    capturablevideosurface.h \
    recognizers/mobilenetssdrecognizer.h \
    recognitionitem.h \
    recognizers/recognizer.h

FORMS    += mainwindow.ui

RESOURCES += \
    res/res.qrc

win32 {
    RC_ICONS = logo.ico
}

TRANSLATIONS += WebCamRecognizer_ru.ts

DEFINES += PROGRAMVERSION=\\\"1.0.0a\\\"

# Comment it to use OpenGL
DEFINES += QT_NO_OPENGL
