#include "controller.h"

Controller::Controller(QRect screenSize)
{
    _screenSize = screenSize;
    _cWorker = new CalibrationWorker(screenSize);
    _cUI = new CalibrationUI(screenSize);
    _imageProcessor = new ImageProcessor();
    _imageProcessingThread = new QThread(this);
    _unlockBlock = new UnlockBlock();
    _unlockBlock->move(0,0);
    _unlockBlock->show();
};

void Controller::setMatchPoint(MatchPoint point, unsigned int i) {
    _matchPoints[i] = point;
    emit match_point_recieved();
}

const int x_pairs[] = {0, 2, 3, 5, 6, 8};
const int y_pairs[] = {0, 6, 1, 7, 2, 8};

void Controller::recomputeDots() {      // учет трех процентов между краями экрана и точками калибровки
    float xSixPercent, ySixPercent;
    int len(sizeof(x_pairs)/sizeof(int));
    for (int i = 0; i < len/2; i++) {
        for (int j = 0; j < 2; j++) {
            xSixPercent = 6.0 * (_matchPoints[x_pairs[i*2]].eyesCoordinates.eyes[j].pupilCoordinates.x - _matchPoints[x_pairs[i*2+1]].eyesCoordinates.eyes[j].pupilCoordinates.x) / 100.0;
            _matchPoints[x_pairs[i*2]].eyesCoordinates.eyes[j].pupilCoordinates.x += xSixPercent/2;
            _matchPoints[x_pairs[i*2+1]].eyesCoordinates.eyes[j].pupilCoordinates.x += xSixPercent/2;

            ySixPercent = 6.0 * (_matchPoints[y_pairs[i*2+1]].eyesCoordinates.eyes[j].pupilCoordinates.y - _matchPoints[y_pairs[i*2]].eyesCoordinates.eyes[j].pupilCoordinates.y) / 100.0;
            _matchPoints[y_pairs[i*2+1]].eyesCoordinates.eyes[j].pupilCoordinates.y += ySixPercent/2;
            _matchPoints[y_pairs[i*2]].eyesCoordinates.eyes[j].pupilCoordinates.y += ySixPercent/2;
        }
    }
}

void Controller::startCalibration() {
    _calibrationThread = new QThread(this);
    _cWorker->moveToThread(_calibrationThread);
    _imageProcessor->moveToThread(_imageProcessingThread);
    // отображение окна калибровки после создания потока
    connect(_calibrationThread, SIGNAL(started()), _cUI, SLOT(showWindow()));
    connect(_calibrationThread, SIGNAL(started()), _cWorker, SLOT(drawFirstDot()));
    // при старте работы потока _imageProcessingThread - начать захват и обработку кадра
    connect(_imageProcessingThread, SIGNAL(started()), _imageProcessor, SLOT(process()));
    // как только кадр обработан - начать следующий цикл обработки нового кадра
    connect(_imageProcessor, SIGNAL(processing_ended()), _imageProcessor, SLOT(process()));
    // модуль CalibrationWorker получить данные обработки ImageProcessor'a
    connect(_imageProcessor, SIGNAL(send_eyes_data(Eyes)), _cWorker, SLOT(recieveEyesData(Eyes)));
    connect(_cWorker, SIGNAL(eyes_data_recieved()), _cWorker, SLOT(processData()));
    // связи воркера и ui (отображение и исчезновение точек на экране)
    connect(_cWorker, SIGNAL(gather_dot()), _cUI, SLOT(gatherDot()));
    connect(_cWorker, SIGNAL(draw_dot(Dot)), _cUI, SLOT(drawDot(Dot)));
    connect(this, SIGNAL(move_cursor(Dot)), _unlockBlock, SLOT(moveCursor(Dot)));

    connect(_cWorker, SIGNAL(stop_calibration()), _cUI, SLOT(closeWindow()));
    // пересылка данных из воркера в контроллер
    connect(_cWorker, SIGNAL(send_data(MatchPoint, unsigned int)), this, SLOT(setMatchPoint(MatchPoint, unsigned int)));
    // после подачи сигнала окончания калибровки воркером - поток->quit
    connect(_cWorker, SIGNAL(stop_calibration()), this, SLOT(recomputeDots()));
    connect(_cWorker, SIGNAL(stop_calibration()), _calibrationThread, SLOT(quit()));
    connect(_cWorker, SIGNAL(stop_calibration()), this, SLOT(startWork()));
    connect(this, SIGNAL(dots_recalibrated()), this, SLOT(startWork()));
    // закрыть окно калибровки после завершения работы потока
    connect(_calibrationThread, SIGNAL(finished()), _cUI, SLOT(closeWindow()));
    connect(_calibrationThread, SIGNAL(finished()), this, SLOT(saveOldData()));

    _imageProcessingThread->start();
    _calibrationThread->start();
}

void Controller::startWork() {
    disconnect(_calibrationThread, SIGNAL(started()), _cWorker, SLOT(drawFirstDot()));
    disconnect(_imageProcessor, SIGNAL(send_eyes_data(Eyes)), _cWorker, SLOT(recieveEyesData(Eyes)));
    disconnect(_cWorker, SIGNAL(stop_calibration()), this, SLOT(recomputeDots()));
    connect(this, SIGNAL(eyes_data_recieved()), this, SLOT(eyesDataProcessing()));
    connect(_imageProcessor, SIGNAL(send_eyes_data(Eyes)), this, SLOT(recieveEyesData(Eyes)));
    connect(this, SIGNAL(start_fast_calibration()), this, SLOT(fastCalibration()));
}

void Controller::recieveEyesData(Eyes e) {
    _eyesData = e;

    if ((e.facePosition.width != 0) && (abs(e.facePosition.x + e.facePosition.width/2 - _centralEyeOldData.facePosition.x - _centralEyeOldData.facePosition.width/2) >= _screenSize.width() * FACE_POSITION_FAULT)) {
        emit start_fast_calibration();
        return;
    }
    if ((e.facePosition.height != 0) && (abs(e.facePosition.y + e.facePosition.height/2 - _centralEyeOldData.facePosition.y - _centralEyeOldData.facePosition.height/2) >= _screenSize.height() * FACE_POSITION_FAULT)) {
       emit start_fast_calibration();
       return;
    }
    emit eyes_data_recieved();
}

int returnFourth(Dot centre, Dot d) {
    int f(0);
    if (d.x > centre.x)    // левая половина
        f = 1;
    else
        f = 2;              // правая половина

    if (d.y > centre.y)     // нижняя половина
        if (f == 1)
            return 3;       // нижняя левая часть
        else
            return 4;       // нижняя правая часть
    else
        if (f == 1)
            return 1;       // верхняя левая чать
        else
            return 2;       // верхняя правая часть
}

Dot * Controller::computeDot(Eyes e) {            // вычислить по координате положениe зрачка положение курсора на экране
    int whichEyeDetected = (e.isEyeDetected == 3 || e.isEyeDetected == 1)? 0 : (e.isEyeDetected == 2)? 1 : 0;
    if (e.isEyeDetected == 0)
        return nullptr;

    int f;
    f = returnFourth(_matchPoints[4].eyesCoordinates.eyes[whichEyeDetected].pupilCoordinates, e.eyes[whichEyeDetected].pupilCoordinates);
    int *mas = new int[4];
    if (f < 3) {
        mas[0] = f-1;
        mas[1] = f;
        mas[2] = f+2;
        mas[3] = f+3;
    }
    else {
        mas[0] = f;
        mas[1] = f+1;
        mas[2] = f+3;
        mas[3] = f+4;
    }

    MatchPoint newCentre = _matchPoints[f+8];
    f = returnFourth(newCentre.eyesCoordinates.eyes[0].pupilCoordinates, e.eyes[whichEyeDetected].pupilCoordinates);
    MatchPoint edgePoint = _matchPoints[mas[f-1]];
    int newCentreX(newCentre.eyesCoordinates.eyes[whichEyeDetected].pupilCoordinates.x),
        edgePointX(edgePoint.eyesCoordinates.eyes[whichEyeDetected].pupilCoordinates.x),
        newCentreY(newCentre.eyesCoordinates.eyes[whichEyeDetected].pupilCoordinates.y),
        edgePointY(edgePoint.eyesCoordinates.eyes[whichEyeDetected].pupilCoordinates.y);
    float ab_x(fabs((newCentreX - e.eyes[whichEyeDetected].pupilCoordinates.x) / (newCentreX - edgePointX))),
            ab_y(fabs((newCentreY - e.eyes[whichEyeDetected].pupilCoordinates.y) / (newCentreY - edgePointY))),
            x, y;
    if (e.eyes[whichEyeDetected].pupilCoordinates.y > newCentreY)       // точка взгляда находится ниже центральной точки
        y = newCentre.screenCoordinates.y + ab_y * fabs(newCentre.screenCoordinates.y - edgePoint.screenCoordinates.y);
    else
        y = newCentre.screenCoordinates.y - ab_y * fabs(newCentre.screenCoordinates.y - edgePoint.screenCoordinates.y);

    if (e.eyes[whichEyeDetected].pupilCoordinates.x > newCentreX)
        x = newCentre.screenCoordinates.x - ab_x * fabs(newCentre.screenCoordinates.x - edgePoint.screenCoordinates.x);
    else
        x = newCentre.screenCoordinates.x + ab_x * fabs(newCentre.screenCoordinates.x - edgePoint.screenCoordinates.x);

    return new Dot(x, y);
}

void Controller::eyesDataProcessing() {         // Определение координат экрана по координатам центров зрачков
    Eyes e = _eyesData;
        if (e.isEyeDetected == 0) {             // если глаза не распознаны, то ничего делать не нужно
            return;                             // временной счетчик / или счетчик проходов - если долгое время не обнаружены глаза - вывести сообщение
        }
        if (_movingList.isEmpty()) {
            _movingList.push_back(e);
            _moving_timer.start();
        }
        else
            if (_moving_timer.elapsed() >= MOVING_DELAY) {
                _movingList.push_back(e);
                Dot d = *(computeDot(CalibrationWorker::countMedian(_movingList)));
                _movingList.clear();
                emit move_cursor(d);
                _moving_timer.start();
            }
        else
            _movingList.push_back(e);
        if (_eyesList.isEmpty()) {
            _eyesList.push_back(e);
            _fixation_timer.start();
        }
        else {
            int len(_eyesList.size()), i(0);
            for (; i < len; i++)
                if (! CalibrationWorker::isEyeFixed(_eyesList[i], e)) {
                    _fixation_timer.start();
                    _eyesList.clear();
                    _eyesList.push_back(e);
                    break;
                }
            if (_fixation_timer.elapsed() >= CONTROL_FIXATION_TIME) {
                _fixation_timer.start();
                Dot d = *(computeDot(CalibrationWorker::countMedian(_eyesList)));
                emit move_cursor(d);
                mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
                _eyesList.clear();
            }
        }
}

void Controller::fastCalibration() {

    disconnect(this, SIGNAL(eyes_data_recieved()), this, SLOT(eyesDataProcessing()));
    disconnect(_imageProcessor, SIGNAL(send_eyes_data(Eyes)), this, SLOT(recieveEyesData(Eyes)));

    // отображение окна калибровки после создания потока
    connect(_calibrationThread, SIGNAL(started()), _cWorker, SLOT(drawCentralDot()));
    connect(_imageProcessor, SIGNAL(send_eyes_data(Eyes)), _cWorker, SLOT(recieveEyesData(Eyes)));
    connect(this, SIGNAL(match_point_recieved()), this, SLOT(calibrateDots()));

    connect(_calibrationThread, SIGNAL(finished()), _cUI, SLOT(closeWindow()));
    connect(_calibrationThread, SIGNAL(finished()), this, SLOT(saveOldData()));
    _calibrationThread->start();
}

void Controller::calibrateDots() {
    float   left_delta_x  = (_centralEyeOldData.eyes[0].pupilCoordinates.x - _matchPoints[4].eyesCoordinates.eyes[0].pupilCoordinates.x) / _centralEyeOldData.eyes[0].pupilCoordinates.x,
            right_delta_x = (_centralEyeOldData.eyes[1].pupilCoordinates.x - _matchPoints[4].eyesCoordinates.eyes[1].pupilCoordinates.x) / _centralEyeOldData.eyes[1].pupilCoordinates.x,
            left_delta_y  = (_centralEyeOldData.eyes[0].pupilCoordinates.y - _matchPoints[4].eyesCoordinates.eyes[0].pupilCoordinates.y) / _centralEyeOldData.eyes[0].pupilCoordinates.y,
            right_delta_y = (_centralEyeOldData.eyes[1].pupilCoordinates.y - _matchPoints[4].eyesCoordinates.eyes[1].pupilCoordinates.y) / _centralEyeOldData.eyes[1].pupilCoordinates.y;
    for (int i = 0; i < 13; i++) {
        if (i != 4) {
            _matchPoints[i].eyesCoordinates.eyes[0].pupilCoordinates.x += _matchPoints[i].eyesCoordinates.eyes[0].pupilCoordinates.x * left_delta_x;
            _matchPoints[i].eyesCoordinates.eyes[1].pupilCoordinates.x += _matchPoints[i].eyesCoordinates.eyes[1].pupilCoordinates.x * right_delta_x;
            _matchPoints[i].eyesCoordinates.eyes[0].pupilCoordinates.y += _matchPoints[i].eyesCoordinates.eyes[0].pupilCoordinates.y * left_delta_y;
            _matchPoints[i].eyesCoordinates.eyes[1].pupilCoordinates.y += _matchPoints[i].eyesCoordinates.eyes[1].pupilCoordinates.y * right_delta_y;
        }
        else {
            _matchPoints[i].eyesCoordinates = _centralEyeOldData;
        }
    }
    disconnect(this, SIGNAL(match_point_recieved()), this, SLOT(calibrateDots()));
    emit dots_recalibrated();
}

void Controller::saveOldData() {
    _centralEyeOldData = _matchPoints[4].eyesCoordinates;
}
