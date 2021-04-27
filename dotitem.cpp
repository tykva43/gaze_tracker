#include "dotitem.h"

DotItem::DotItem(QObject *parent) :
    QObject(parent), QGraphicsItem()
{};

DotItem::~DotItem() {

};

void DotItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    painter->setPen(lightColor);
    painter->setBrush(lightColor);
    painter->drawEllipse(0, 0, DOT_RADIUS, DOT_RADIUS);
    Q_UNUSED(option);
    Q_UNUSED(widget);
};

QRectF DotItem::boundingRect() const
{
    return QRectF (0, 0, DOT_RADIUS, DOT_RADIUS);
}
