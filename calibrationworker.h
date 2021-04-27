#ifndef CALIBRATIONWORKER_H
#define CALIBRATIONWORKER_H

#include <QObject>
#include <QDebug>
#include <QTime>
#include <QRect>

#include <imageprocessor.h>
#include <eyes.h>

#define DOT_EDGE_CORRECTION 0.03  // расстояние от края экрана до точек калибровки, отмеченных по периметру

#define CALIBRATION_FIXATION_TIME 3000 // время в милисекундах фиксации взгляда для калибровки
#define SACCADE_FAULT 10 // погрешность
#define DOT_NUMBER 13

class CalibrationWorker : public QObject
{
    Q_OBJECT
public:
    CalibrationWorker(QRect);
    MatchPoint *getMatchPoints();
    static bool isEyeFixed(Eyes, Eyes);
    static Eyes countMedian(QList<Eyes>);

private:
    MatchPoint _matchPoints[DOT_NUMBER];
    ImageProcessor * _imageProcessor;
    Eyes _eyesData;
    int currentDotNumber = 0;
    QTime _timer;
    QList<Eyes> _eyesList;
    int counter;
    int calibrationMode = 1; // 1 - first calibration, 2 and more - fast recalibration

public slots:
    void stop();
    void showAllDots();
    void recieveEyesData(Eyes);
    void processData();
    void drawFirstDot();
    void drawCentralDot();

signals:
    void stop_calibration();
    void gather_dot();
    void draw_dot(Dot);
    void send_data(MatchPoint, unsigned int);
    void eyes_data_recieved();
};

#endif // CALIBRATIONWORKER_H
