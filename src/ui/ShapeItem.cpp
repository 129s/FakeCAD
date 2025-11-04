#include "ShapeItem.h"

#include <QPainter>

ShapeItem::ShapeItem(std::unique_ptr<Shape> shape, QGraphicsItem* parent)
    : QGraphicsItem(parent), shape_(std::move(shape)) {
    setFlag(ItemIsSelectable, true);
    setFlag(ItemIsMovable, true);
    // 使用模型中的平移初始化项位置
    const auto& t = shape_->transform();
    setPos(t.m31(), t.m32());
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
        const double L = ls->Length();
        painter->setPen(QPen(Qt::darkGray));
        painter->drawText(boundingRect().translated(4, -4).topLeft(), QString("L=%1").arg(L, 0, 'f', 2));
        return;
    }
    if (auto* rc = dynamic_cast<Rectangle*>(shape_.get())) {
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(rc->rect());
        painter->setPen(QPen(Qt::darkGray));
        const double P = rc->Perimeter();
        const double A = rc->Area();
        painter->drawText(rc->rect().topLeft() + QPointF(4, 12), QString("P=%1 A=%2").arg(P, 0, 'f', 2).arg(A, 0, 'f', 2));
        return;
    }
    if (auto* cc = dynamic_cast<Circle*>(shape_.get())) {
        painter->setBrush(Qt::NoBrush);
        const auto r = cc->radius();
        painter->drawEllipse(cc->center(), r, r);
        painter->setPen(QPen(Qt::darkGray));
        const double P = cc->Perimeter();
        const double A = cc->Area();
        painter->drawText(boundingRect().translated(4, -4).topLeft(), QString("P=%1 A=%2").arg(P, 0, 'f', 2).arg(A, 0, 'f', 2));
        return;
    }
}
