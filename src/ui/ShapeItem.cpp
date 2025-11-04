#include "ShapeItem.h"

#include <QPainter>

ShapeItem::ShapeItem(std::unique_ptr<Shape> shape, QGraphicsItem* parent)
    : QGraphicsItem(parent), shape_(std::move(shape)) {
    setFlag(ItemIsSelectable, true);
    setFlag(ItemIsMovable, false); // 移动交互后续开启
}

QRectF ShapeItem::boundingRect() const {
    if (auto* ls = dynamic_cast<LineSegment*>(shape_.get())) {
        return QRectF(ls->p1(), ls->p2()).normalized().adjusted(-1, -1, 1, 1);
    }
    if (auto* rc = dynamic_cast<Rectangle*>(shape_.get())) {
        return rc->rect().normalized().adjusted(-1, -1, 1, 1);
    }
    if (auto* cc = dynamic_cast<Circle*>(shape_.get())) {
        const auto r = cc->radius();
        return QRectF(cc->center().x() - r, cc->center().y() - r, 2 * r, 2 * r).adjusted(-1, -1, 1, 1);
    }
    return {};
}

void ShapeItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(shape_->pen());

    if (auto* ls = dynamic_cast<LineSegment*>(shape_.get())) {
        painter->drawLine(ls->p1(), ls->p2());
        return;
    }
    if (auto* rc = dynamic_cast<Rectangle*>(shape_.get())) {
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(rc->rect());
        return;
    }
    if (auto* cc = dynamic_cast<Circle*>(shape_.get())) {
        painter->setBrush(Qt::NoBrush);
        const auto r = cc->radius();
        painter->drawEllipse(cc->center(), r, r);
        return;
    }
}

