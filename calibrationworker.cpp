#include "calibrationworker.h"

int correctEdgeDot(int x, int max_x) {      // коррекция координат точек периметра экрана
    if (x == 0)
        x = floor(DOT_EDGE_CORRECTION * max_x);
    else if (abs(x - max_x) <= 5)
        x = floor(max_x * (1.0 - DOT_EDGE_CORRECTION));
    return x;
}

CalibrationWorker::CalibrationWorker(QRect screenSize)
{
    int xstep = floor(screenSize.width()/2),
        ystep = floor(screenSize.height()/2);


    for (int i = 0; i < 9; i++) {
        _matchPoints[i].screenCoordinates.x = correctEdgeDot((i % 3)*xstep, screenSize.width());
        _matchPoints[i].screenCoordinates.y = correctEdgeDot((i / 3)*ystep, screenSize.height());
    }

    xstep = floor(screenSize.width()/4);
    ystep = floor(screenSize.height()/4);

    _matchPoints[9].screenCoordinates.x = xstep;     _matchPoints[9].screenCoordinates.y = ystep;
    _matchPoints[10].screenCoordinates.x = 3*xstep;  _matchPoints[10].screenCoordinates.y = ystep;
    _matchPoints[11].screenCoordinates.x = xstep;    _matchPoints[11].screenCoordinates.y = 3*ystep;
    _matchPoints[12].screenCoordinates.x = 3*xstep;  _matchPoints[12].screenCoordinates.y = 3*ystep;

    currentDotNumber = 0;
    calibrationMode = 1;
}


void CalibrationWorker::showAllDots() {
    for (int i = 0; i < 13; i++) {
        emit draw_dot(_matchPoints[i].screenCoordinates);
    }
}

MatchPoint * CalibrationWorker::getMatchPoints() {
    return _matchPoints;
};

// функция для сортировки данных Eyes
bool caseInsensitiveForXCoord(const Eyes &e1, const Eyes &e2)
{
    if (e1.isEyeDetected == 3 && e2.isEyeDetected == 3)
        return  e1.eyes[0].pupilCoordinates.x < e2.eyes[0].pupilCoordinates.x;
    if (((e1.isEyeDetected == 3) || (e1.isEyeDetected == 1)) && ((e2.isEyeDetected == 3) || (e2.isEyeDetected == 1)))
        return  e1.eyes[0].pupilCoordinates.x < e2.eyes[0].pupilCoordinates.x;
    if (((e1.isEyeDetected == 3) || (e1.isEyeDetected == 2)) && ((e2.isEyeDetected == 3) || (e2.isEyeDetected == 2)))
        return  e1.eyes[1].pupilCoordinates.x < e2.eyes[1].pupilCoordinates.x;
    return false;
}

bool caseInsensitiveForYCoord(const Eyes &e1, const Eyes &e2)
{
    if (e1.isEyeDetected == 3 && e2.isEyeDetected == 3)
        return  e1.eyes[0].pupilCoordinates.y < e2.eyes[0].pupilCoordinates.y;
    if (((e1.isEyeDetected == 3) || (e1.isEyeDetected == 1)) && ((e2.isEyeDetected == 3) || (e2.isEyeDetected == 1)))
        return  e1.eyes[0].pupilCoordinates.y < e2.eyes[0].pupilCoordinates.y;
    if (((e1.isEyeDetected == 3) || (e1.isEyeDetected == 2)) && ((e2.isEyeDetected == 3) || (e2.isEyeDetected == 2)))
        return  e1.eyes[1].pupilCoordinates.y < e2.eyes[1].pupilCoordinates.y;
    return false;
}

Eyes CalibrationWorker::countMedian(QList<Eyes> list) {       // подсчет медианы среди координат полученых массивов
    qSort(list.begin(), list.end(), caseInsensitiveForYCoord);
    Eyes e = list[list.length()/2];
    return e;
}

bool CalibrationWorker::isEyeFixed(Eyes fixedEyes, Eyes newEyes) {
    if (fabs(fixedEyes.eyes[0].pupilCoordinates.x - newEyes.eyes[0].pupilCoordinates.x) <= SACCADE_FAULT)
        return true;
    else
        return false;
}

void CalibrationWorker::recieveEyesData(Eyes e) {
    _eyesData = e;
    emit eyes_data_recieved();
}

void CalibrationWorker::processData() {
    Eyes e = _eyesData;
    if (e.isEyeDetected != 3) {
        return;
    }
    if (_eyesList.isEmpty()) {
        _eyesList.push_back(e);
        _timer.start();
        return;
    }
    else {
        int len(_eyesList.length()), i(0);
        for (; i < len; i++)
            if (! CalibrationWorker::isEyeFixed(_eyesList[i], e)) {
                _eyesList.clear();
                _eyesList.push_back(e);
                _timer.start();
                return;
            }
    }
    _eyesList.push_back(e);
    if (_timer.elapsed() >= CALIBRATION_FIXATION_TIME) {
        _matchPoints[currentDotNumber].eyesCoordinates = countMedian(_eyesList);
        _eyesList.clear();
        emit gather_dot();
        emit send_data(_matchPoints[currentDotNumber], currentDotNumber);
        currentDotNumber++;
        if (((calibrationMode == 1) && (currentDotNumber > DOT_NUMBER - 1)) || (calibrationMode > 1)) {
            emit stop_calibration();
            calibrationMode++;
            return;
        }
        emit draw_dot(_matchPoints[currentDotNumber].screenCoordinates);
    }
}

void CalibrationWorker::drawFirstDot() {
     emit draw_dot(_matchPoints[0].screenCoordinates);
}

void CalibrationWorker::drawCentralDot() {
    currentDotNumber = 4;
    emit draw_dot(_matchPoints[currentDotNumber].screenCoordinates);
}

void CalibrationWorker::stop() {
    qDebug() << "Calibration stopped";
}
