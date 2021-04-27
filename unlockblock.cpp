#include "unlockblock.h"
#include "ui_unlockblock.h"

UnlockBlock::UnlockBlock(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UnlockBlock)
{
    ui->setupUi(this);
    ui->unlockButton->setIcon(QPixmap(":/images/resources/eye_visible.ico"));       // установка иконки на кнопку
    ui->unlockButton->setIconSize(ui->unlockButton->size());
    this->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint);                 // без рамки и поверх всех окон
    this->hide();
    this->isVisible = 1;
}

UnlockBlock::~UnlockBlock()
{
    delete ui;
}

void UnlockBlock::showWindow(QPoint point) {
    this->move(point);
    this->show();
}

void UnlockBlock::closeWindow() {
    this->close();
}

void UnlockBlock::unlockButtonClicked() {
    if (isVisible) {
        ui->unlockButton->setIcon(QPixmap(":/images/resources/eye_unvisible.ico"));
    }
    else {
        ui->unlockButton->setIcon(QPixmap(":/images/resources/eye_visible.ico"));
    }
    isVisible = !isVisible;
    ui->unlockButton->setIconSize(ui->unlockButton->size());
}

void UnlockBlock::moveCursor(Dot newDot) {

    Dot currentDot(QCursor::pos().x(), QCursor::pos().y());

    int xDelta = (newDot.x - currentDot.x)/100,
          yDelta = (newDot.y - currentDot.y)/100;
    int xn = currentDot.x;
    int yn = currentDot.y;

    for (int i = 0; i < 100; i++) {
        QCursor::setPos(xn, yn);
        xn += xDelta;
        yn += yDelta;
    }
}
