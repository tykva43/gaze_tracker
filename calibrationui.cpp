#include "calibrationui.h"
#include "ui_calibrationui.h"

CalibrationUI::CalibrationUI(QRect screenSize) :
    QWidget(),
    ui(new Ui::CalibrationUI)
{
    ui->setupUi(this);
    _scene = new QGraphicsScene(this);                                  // Инициализируем графическую сцену
    _scene->setItemIndexMethod(QGraphicsScene::NoIndex);                // настраиваем индексацию элементов
    ui->graphicsView->setScene(_scene);                                 // Устанавливаем графическую сцену в graphicsView
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);            // Настраиваем рендер
    ui->graphicsView->setCacheMode(QGraphicsView::CacheBackground);     // Кэш фона
    ui->graphicsView->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    _scene->setSceneRect(0, 0, screenSize.width(), screenSize.height()); // Устанавливаем размер сцены
    this->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint);                 // без рамки и поверх всех окон
}

void CalibrationUI::closeWindow() {
    this->close();
}

void CalibrationUI::showWindow() {
    this->showMaximized();
}

CalibrationUI::~CalibrationUI()
{
    delete ui;
}

void CalibrationUI::drawDot(Dot dot) {
    _item = new DotItem();        // Создаём графический элемент
    _item->setPos(dot.x, dot.y);
    _scene->addItem(_item);
};

void CalibrationUI::gatherDot() {
    _scene->removeItem(_item);
};
