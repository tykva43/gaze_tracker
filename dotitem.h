#ifndef DOTITEM_H
#define DOTITEM_H

#include <QObject>
#include <QGraphicsItem>
#include <QPainter>
#include <QColor>
#include <QTimeLine>
#include <QGraphicsItemAnimation>

#include <eyes.h>

#define DOT_RADIUS 10

const QColor lightColor(255, 255, 196);

class DotItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
public:
    explicit DotItem(QObject *parent = 0);
    ~DotItem();

signals:

private:
    QRectF boundingRect() const;
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);
};

#endif // DOTITEM_H
