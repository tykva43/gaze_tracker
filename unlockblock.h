#ifndef UNLOCKBLOCK_H
#define UNLOCKBLOCK_H

#include <QWidget>
#include <QDebug>

#include "eyes.h"

namespace Ui {
class UnlockBlock;
}

class UnlockBlock : public QWidget
{
    Q_OBJECT

public:
    explicit UnlockBlock(QWidget *parent = nullptr);
    ~UnlockBlock();

public slots:
    void showWindow(QPoint);
    void closeWindow();
    void unlockButtonClicked();
    void moveCursor(Dot);

private:
    Ui::UnlockBlock *ui;
    bool isVisible;
};

#endif // UNLOCKBLOCK_H
