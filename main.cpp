#include <iostream>
#include <QDir>
#include <QApplication>
#include <QDebug>
#include <QDate>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/videoio.hpp>

#include <imageprocessor.h>
#include <unlockblock.h>
#include <controller.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qRegisterMetaType<Dot> ("Dot");
    qRegisterMetaType<MatchPoint>("MatchPoint");
    qRegisterMetaType<Eyes>("Eyes");
    QRect screenSize = (QApplication::screens())[0]->geometry();
    Controller *controller = new Controller(screenSize);
    controller->startCalibration();
    return a.exec();
}

