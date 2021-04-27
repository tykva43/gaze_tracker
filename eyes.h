#pragma once

#include <opencv2/opencv.hpp>

#include <QPoint>

#ifndef EYES_H
#define EYES_H

typedef struct Dot {
public:
    float x, y;
    void operator=(Dot inDot) {
        this->x = inDot.x;
        this->y = inDot.y;
    }

    QPoint toQPoint() {
        return QPoint(int(this->x), int(this->y));
    }

    void clear() {
        this->x = 0;
        this->y = 0;
    }
    Dot(float _x, float _y) {
        this->x = _x;
        this->y = _y;
    }

    Dot() { }

} Dot;

typedef struct Eye {
public:
    double pupilDiameter;
    Dot pupilCoordinates;
    cv::Rect eyePosition;



    void operator=(Eye inEye) {
        this->pupilDiameter = inEye.pupilDiameter;
        this->pupilCoordinates = inEye.pupilCoordinates;
        this->eyePosition = inEye.eyePosition;
    }

    void clear() {
        this->pupilDiameter = 0;
        this->pupilCoordinates.clear();
    }

    Eye() {}

} Eye;


typedef struct Eyes {
public:
    cv::Rect facePosition;          // параметры расположения лица относительно всего кадра
    Eye eyes[2];                    // параметры расположения глаза относительно кадра лица
    double interpupillaryDistance;  // межзрачковое расстояние
    int isEyeDetected;              // 0 - ни один глаз нераспознан, 1 - левый распознан, 2 - правый распознан, 3 - оба распознаны

    void operator=(Eyes inEyes) {
        this->eyes[0] = inEyes.eyes[0];
        this->eyes[1] = inEyes.eyes[1];
        this->facePosition = inEyes.facePosition;
        this->interpupillaryDistance = inEyes.interpupillaryDistance;
        this->isEyeDetected = inEyes.isEyeDetected;
    }

    void clear() {
        this->eyes[0].clear();
        this->eyes[1].clear();
        this->interpupillaryDistance = 0;
    }
    explicit Eyes() {
        this->clear();
    }

} Eyes;

typedef struct MatchPoint{
public:
    Dot screenCoordinates;
    Eyes eyesCoordinates;
    MatchPoint(){ }
} MatchPoint;


#endif // EYES_H


