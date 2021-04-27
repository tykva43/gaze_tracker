#include <imageprocessor.h>
#include <calibrationui.h>
#include <calibrationworker.h>
#include <unlockblock.h>

#include "windows.h"

#include <QTime>
#include <QObject>
#include <QThread>

#ifndef CONTROLLER_H
#define CONTROLLER_H

#define CONTROL_FIXATION_TIME 3000
#define MOVING_DELAY 200
#define FACE_POSITION_FAULT 0.05

class Controller : public QObject
{
    Q_OBJECT
public:
    explicit Controller(QRect);

public slots:
    void setMatchPoint(MatchPoint, unsigned int);
    void startCalibration();
    void recieveEyesData(Eyes);
    void startWork();
    void eyesDataProcessing();
    Dot *computeDot(Eyes);
    void recomputeDots();
    void calibrateDots();
    void fastCalibration();
    void saveOldData();

private:
    QTime _fixation_timer, _moving_timer;
    CalibrationWorker * _cWorker;
    CalibrationUI * _cUI;
    QThread * _imageProcessingThread;
    QThread * _calibrationThread;
    ImageProcessor *_imageProcessor;
    UnlockBlock * _unlockBlock;
    MatchPoint _matchPoints[13];
    QRect _screenSize;
    Eyes _eyesData;
    Eyes _centralEyeOldData;
    QList<Eyes> _movingList;
    QList<Eyes> _eyesList;

signals:
    void stop_image_processing();
    void stop_calibration();
    void start_calibration();
    void eyes_data_recieved();
    void mouse_click();
    void double_mouse_click();
    void head_position_changed();
    void move_cursor(Dot);
    void match_point_recieved();
    void dots_recalibrated();
    void start_fast_calibration();
};

#endif // CONTROLLER_H
