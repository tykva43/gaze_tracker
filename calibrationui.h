#ifndef CALIBRATIONUI_H
#define CALIBRATIONUI_H

#include <QWidget>
#include <QGraphicsScene>
#include <QScreen>
#include <QDebug>

//#include "windows.h"

#include <eyes.h>
#include <dotitem.h>

namespace Ui { class CalibrationUI; }

class CalibrationUI :  public QWidget
{
    Q_OBJECT

public:
    explicit CalibrationUI(QRect);
    ~CalibrationUI();

private:
    Ui::CalibrationUI * ui;
    QGraphicsScene * _scene;
    DotItem * _item;

public slots:
    void drawDot(Dot);
    void gatherDot();
    void showWindow();
    void closeWindow();
};

#endif // CALIBRATIONUI_H
