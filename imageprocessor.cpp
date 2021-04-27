#include "imageprocessor.h"

#include <QDebug>

ImageProcessor::ImageProcessor() : QObject()
{
    QTemporaryDir tempDir;  std::string path;
    if (tempDir.isValid()) {
        const QString tempFile_face = tempDir.path() + "/haarcascade_frontalface_alt2.xml";
        const QString tempFile_eye = tempDir.path() + "/haarcascade_eye.xml";
        if (QFile::copy(":/cascades/resources/haarcascade_frontalface_alt2.xml", tempFile_face)) {
            path = tempFile_face.toLocal8Bit().constData();
            if (!_face_cascade.load(path)) {
                emit cascades_is_not_loaded();
            }
        }
        if (QFile::copy(":/cascades/resources/haarcascade_eye.xml", tempFile_eye)) {
            path = tempFile_eye.toLocal8Bit().constData();
            if (!_eye_cascade.load(path)) {
                emit cascades_is_not_loaded();
            }
        }
    }

    // настраиваем объект camera для работы с веб-камерой
    _camera.open(2);

    // проверяем, удалось ли подключиться
    if (!_camera.isOpened()) {
        emit camera_is_not_connected();
        qDebug() << "Camera is not opened";
        return;
    }

    height = 0;
    width = 0;
    // выставляем параметры камеры ширину и высоту кадра
    setMaxCameraResolution(_camera);
}

cv::Mat ImageProcessor::getFrameRedChannel(cv::Mat image) {
    cv::Mat rgbChannels[3];
    cv::split(image, rgbChannels);
    return rgbChannels[2];
};

bool ImageProcessor::isCameraOpened() {
    return _camera.isOpened();
}

void ImageProcessor::setMaxCameraResolution(cv::VideoCapture & camera) {
    int len(CommonResolutions.size()), i(len-1);
    for (; i >= 0; i--) {
        _sleep(150);
        camera.set(cv::CAP_PROP_FRAME_WIDTH, CommonResolutions[i].width);
        camera.set(cv::CAP_PROP_FRAME_HEIGHT, CommonResolutions[i].height);

        if ((camera.get(cv::CAP_PROP_FRAME_WIDTH) == CommonResolutions[i].width) && (camera.get(cv::CAP_PROP_FRAME_HEIGHT) == CommonResolutions[i].height))
        {
            break;
        }
    }
}


cv::Mat ImageProcessor::binarizeImage(cv::Mat inFrame) {

    cv::Mat outFrame;

    cv::GaussianBlur(inFrame, inFrame, cv::Size(7,7), 0);
    //cv::imshow("GaussianBlur", inFrame);
    inFrame.convertTo(inFrame, -1, 2, 0);       // increase the contrast by 2
    //cv::imshow("Contrast", inFrame);
    int w(inFrame.size().width - EYE_CORRECTION_LEN), h(inFrame.size().height - EYE_CORRECTION_LEN);

    float min_mean(w * h * 255);

    for (int i = 0; i < w; i++ ) {
        for (int j = 0; j < h; j++) {
            float mean(0); int counter(0);
            for (int k = i; k < EYE_CORRECTION_LEN + i; k++) {
                for (int n = j; n < EYE_CORRECTION_LEN + j; n++) {
                    mean += *(inFrame.data + n*inFrame.step + k);
                    counter++;
                }
            }
            mean = mean*1.0/counter;
            if (mean < min_mean)
                min_mean = mean;
        }
    }

    cv::threshold(inFrame, outFrame, LIGHT_PARAMETER*int(min_mean), 255, cv::THRESH_BINARY);
    //cv::imshow("Threshold", outFrame);
    return outFrame;
};



cv::RotatedRect ImageProcessor::ellipsesAproximaton(cv::Mat inFrame) {

    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;

    // *********************  применяем морфологические операции  *********************
    // - эрозию и дилиацию, для того, чтобы убрать лишниепикселы и дополнить недостающие
    cv::dilate(inFrame, inFrame, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5,5)));
    //cv::imshow("After Dilating", inFrame);
    cv::erode(inFrame, inFrame, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3,3)));
    //cv::imshow("After Eroding", inFrame);

    // *********************  находим контур с спомощью детектора Кэнни  *********************
    cv::Canny(inFrame, inFrame, 0, 255);

    // *********************  находим контуры (используется алгоритм Suzuki85)  *********************
    cv::findContours(inFrame, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    // *********************  апроксимируем контур зрачка эллипсом (исп. алгоритм Fitzgibbon95)  *********************
    //! todo: if (contours.isEmpty()) ...
    if (contours.size()>0) {
        unsigned int i(0);
        for (; i < contours.size(); i++)
            if (contours[i].size() >= 5) {
                cv::RotatedRect ellipse = cv::fitEllipse(contours[i]);
                //! todo: emmit some_signal
                return ellipse;
            }
    }
};

Eye * ImageProcessor::getPupilsCoordinates(cv::Mat frame, cv::Rect eyesRect, int leftOrRight = -1) {
    cv::Mat EyeImage = cv::Mat(frame, eyesRect);
    Eye *eye = new Eye();
    if (!EyeImage.empty()) {

        cv::Mat binarizedImg = binarizeImage(EyeImage);

        cv::RotatedRect ellipse = ellipsesAproximaton(binarizedImg);

        eye->pupilCoordinates.x = ellipse.center.x;
        eye->pupilCoordinates.y = ellipse.center.y;
        eye->pupilCoordinates.x += eyesRect.x;
        eye->pupilCoordinates.y += eyesRect.y;
        eye->pupilDiameter = ellipse.size.width;

        //! Демонстрация
/*        cv::ellipse(EyeImage, ellipse, cv::Scalar(255, 255, 255), 2);
        cv::circle(EyeImage, cv::Point(eye->pupilCoordinates.x, eye->pupilCoordinates.y), 1, cv::Scalar(255, 255, 255), 1);
        if (leftOrRight == 0) {
            imshow( "lEye_puilDetection", EyeImage);
        }
        else if (leftOrRight == 1) {
            imshow( "rEye_puilDetection", EyeImage);
        }
*/
        return eye;
    }
    return nullptr;
};



void ImageProcessor::process() {
    QTime t1; t1.start();
    _camera >> _frame;
    //cv::imshow("InFrame", _frame);

    _newEyes.isEyeDetected = eyesDetection(_frame);
    // координаты центра зрачка относительно кадра в целом
    _newEyes.eyes[0].eyePosition.x += _newEyes.facePosition.x + _newEyes.facePosition.width/2 - 1; //+ _newEyes.facePosition.width/2 - 1;
    _newEyes.eyes[0].eyePosition.y += _newEyes.facePosition.y;
    _newEyes.eyes[1].eyePosition.x += _newEyes.facePosition.x;
    _newEyes.eyes[1].eyePosition.y += _newEyes.facePosition.y;

    if (_newEyes.isEyeDetected == 3) {

        _newEyes.eyes[0] = *(getPupilsCoordinates(_frame, _newEyes.eyes[0].eyePosition, 0));

        _newEyes.eyes[1] = *(getPupilsCoordinates(_frame, _newEyes.eyes[1].eyePosition, 1));

    } else if (_newEyes.isEyeDetected == 1) {

        _newEyes.eyes[0] = *(getPupilsCoordinates(_frame, _newEyes.eyes[0].eyePosition, 0));

    } else if (_newEyes.isEyeDetected == 2) {

        _newEyes.eyes[1] = *(getPupilsCoordinates(_frame, _newEyes.eyes[1].eyePosition, 1));

    } else if (_newEyes.isEyeDetected == 0) {
        emit eyes_is_not_detected();
        emit processing_ended();
        return;
    }
    emit send_eyes_data(this->_newEyes);
    emit processing_ended();
};

void returnOriginSize(cv::Rect * r, int parameter) {
    r->x *= parameter;
    r->y *= parameter;
    r->width *= parameter;
    r->height *= parameter;
}

int ImageProcessor::eyesDetection(cv::Mat &frame) {

    std::vector<cv::Rect> faces,                        // вектор для сохранения распознанных на кадре лиц
                          lEyes, rEyes;                 // векторa для сохранения распознанных на кадре глаз
// *********************  создаем полутоновое изображение кадра  *********************
    frame = getFrameRedChannel(frame);

// *********************  распознавание лица  *********************
    cv::Mat resized_frame;
    float reduce_parameter = 1.0 * frame.size().width / RESIZED_WIDTH;
    cv::resize(frame, resized_frame, cv::Size(RESIZED_WIDTH, int(frame.size().height / reduce_parameter)));    // сжимаем изображение

    _face_cascade.detectMultiScale(resized_frame, faces, 1.2, 3, 0, cv::Size(40, 60));
    // если не удалось распознать лицо
    if ( detectionOutputFilter(faces, &_newEyes.facePosition) == false ) {
        emit face_is_not_detected();    // отправляем сигнал контроллеру
        return 0;
    }

    cv::Mat fr(resized_frame);
    //cv::rectangle(fr, faces[0], cv::Scalar(255, 0, 0));
    //imshow( "FaceRecognition", fr);

    int h(_newEyes.facePosition.height), w(_newEyes.facePosition.width);

    cv::Mat rFaceImage = cv::Mat(resized_frame, cv::Rect(_newEyes.facePosition.x, _newEyes.facePosition.y, w/2, 1.7*h/3));
    cv::Mat lFaceImage = cv::Mat(resized_frame, cv::Rect(_newEyes.facePosition.x + w/2 - 1, _newEyes.facePosition.y, w/2, 1.7*h/3));

    _eye_cascade.detectMultiScale(lFaceImage, lEyes, 1.1, 3, 1, cv::Size(10, 10));

    _eye_cascade.detectMultiScale(rFaceImage, rEyes, 1.1, 3, 1, cv::Size(10, 10));

    lFaceImage.copyTo(fr);
    //cv::rectangle(fr, lEyes[0], cv::Scalar(255, 0, 0));
    //imshow( "lEye", fr);

    rFaceImage.copyTo(fr);
    //cv::rectangle(fr, rEyes[0], cv::Scalar(255, 0, 0));
    //imshow( "rEye", fr);

    bool isEyeDetected[2];          // isEyeDetected[0] - left eye, isEyeDetected[1] - right eye
    isEyeDetected[0] = detectionOutputFilter(lEyes, &_newEyes.eyes[0].eyePosition);
    isEyeDetected[1] = detectionOutputFilter(rEyes, &_newEyes.eyes[1].eyePosition);

    returnOriginSize(&(_newEyes.facePosition), reduce_parameter);
    returnOriginSize(&(_newEyes.eyes[0].eyePosition), reduce_parameter);
    returnOriginSize(&(_newEyes.eyes[1].eyePosition), reduce_parameter);

//! Демонстрация
  /*  if (width == 0)
        width = _newEyes.facePosition.width;
    if (height == 0)
        height = _newEyes.eyes[0].eyePosition.height;
    //cv::imshow("Eyes", cv::Mat(frame, cv::Rect(_newEyes.facePosition.x, _newEyes.facePosition.y, width, height)));
*/
    return (isEyeDetected[0] && isEyeDetected[1])? 3 : (isEyeDetected[0] && !isEyeDetected[1])? 1 : (!isEyeDetected[0] && isEyeDetected[1])? 2 : 0;
};




bool ImageProcessor::detectionOutputFilter(std::vector<cv::Rect> detectedObjVector, cv::Rect *detectedObj) {
    if (detectedObjVector.size() > 1) {
        for ( unsigned int i = 0; i < detectedObjVector.size(); i++ ) {
            if ( ( detectedObjVector[i].width > 0 ) && ( detectedObjVector[i].height > 0 ) ) {
                *detectedObj = detectedObjVector[i];
                return true;
            }
            else if ( i == detectedObjVector.size() - 1 ) {
                detectedObj = nullptr;
                return false;
            }
        }
    }
    else {
        if ( (detectedObjVector.size() == 1) && (detectedObjVector[0].width > 0) && (detectedObjVector[0].height > 0) ) {
            *detectedObj = detectedObjVector[0];
            return true;
        }
        else {          // если ни одного объекта не найдено
            detectedObj = nullptr;
            return false;
        }
    }
};

Eyes ImageProcessor::getEyesParameters() {
    return _newEyes;
};

ImageProcessor::~ImageProcessor() {

};

void ImageProcessor::stop() {
    qDebug() << "imageProcessorStopped";
}
