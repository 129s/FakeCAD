#include "LineSegment.h"

#include <QTransform>

LineSegment::LineSegment(const QPointF& p1, const QPointF& p2)
    : p1_(p1), p2_(p2) {
    ++kCount;
}

LineSegment::~LineSegment() { --kCount; }

QRectF LineSegment::BoundingBox() const {
    QRectF localRect = QRectF(p1_, p2_).normalized();
    return transform().mapRect(localRect);
}

