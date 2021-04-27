QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    calibrationui.cpp \
    calibrationworker.cpp \
    controller.cpp \
    dotitem.cpp \
    main.cpp \
    imageprocessor.cpp \
    unlockblock.cpp

HEADERS += \
    calibrationui.h \
    calibrationworker.h \
    controller.h \
    dotitem.h \
    eyes.h \
    imageprocessor.h \
    unlockblock.h

INCLUDEPATH += "C:/openCV_4.1/build/install/include/"

LIBS += -L"C:/openCV_4.1/build/install/x86/mingw/lib/"
LIBS += \
-lopencv_core410d \
-lopencv_highgui410d \
-lopencv_imgproc410d \
-lopencv_photo410d \
-lopencv_highgui410d.dll \
-lopencv_imgcodecs410d \
-lopencv_video410d \
-lopencv_videoio410d \
-lopencv_objdetect410d

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

FORMS += \
    calibrationui.ui \
    unlockblock.ui

RESOURCES += \
    resources.qrc

