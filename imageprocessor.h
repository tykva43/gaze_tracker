#pragma once

#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <QObject>
#include <QTemporaryDir>
#include <QTime>

#include <opencv2/opencv.hpp>

#include <eyes.h>

#define EYE_CORRECTION_LEN 10
#define LIGHT_PARAMETER 1.2
#define RESIZED_WIDTH 320

const QList<cv::Size> CommonResolutions = {
    cv::Size(320, 240),
    cv::Size(352, 240),
    cv::Size(352, 288),
    cv::Size(640, 240),
    cv::Size(640, 360),
    cv::Size(640, 480),
    cv::Size(800, 480),
    cv::Size(800, 600),
    cv::Size(848, 480),
    cv::Size(960, 540),
    cv::Size(1024, 600),
    cv::Size(1024, 768),
    cv::Size(1152, 864),
    cv::Size(1200, 600),
    cv::Size(1280, 720),
    cv::Size(1280, 768),
    cv::Size(1280, 1024),
    cv::Size(1600, 900),
    cv::Size(1920, 1080)
};

class ImageProcessor : public QObject
{
    Q_OBJECT

private:
    cv::Mat _frame;                             // исходный кадр
    Eyes _newEyes;

    cv::CascadeClassifier _face_cascade, _eye_cascade;
    cv::VideoCapture _camera;

    cv::Mat dEyesFrame;
    unsigned int width = 0;
    unsigned int height = 0;

    // методы по обработке изображения для выявления положения зрачка
    cv::Mat getFrameRedChannel(cv::Mat);    // получение красного канала изображения
                                            // для лучшей контрасности зрачка с остальным фоном
    cv::Mat binarizeImage(cv::Mat);         // бинаризация изображения с учетом освещенности лица в кадре
    int eyesDetection(cv::Mat&);            // обнаружение глаз на снимке
    bool detectionOutputFilter(std::vector<cv::Rect>, cv::Rect *);  // фильтр ненужных данных после распознавания лица или глаз
    cv::RotatedRect ellipsesAproximaton(cv::Mat);   // апроксимация эллипсом
    Eye * getPupilsCoordinates(cv::Mat, cv::Rect, int);
    void setMaxCameraResolution(cv::VideoCapture&);

public:
    ImageProcessor();
    Eyes getEyesParameters();
    ~ImageProcessor();
    bool isCameraOpened();

signals:
    void face_is_not_detected();
    void eyes_is_not_detected();
    void one_eye_is_not_detected(bool isRight);
    void camera_is_not_connected();
    void room_is_dark();
    void send_eyes_data(Eyes);
    void cascades_is_not_loaded();
    void processing_ended();

public slots:
    void process();
    void stop();
};

#endif // IMAGEPROCESSOR_H
