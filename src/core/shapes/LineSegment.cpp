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

QJsonObject LineSegment::ToJson() const {
    QJsonObject obj = Shape::ToJson();
    obj["type"] = QStringLiteral("LineSegment");
    obj["geom"] = QJsonObject{{"x1", p1_.x()}, {"y1", p1_.y()}, {"x2", p2_.x()}, {"y2", p2_.y()}};
    return obj;
}

std::unique_ptr<LineSegment> LineSegment::FromJson(const QJsonObject& obj) {
    auto geom = obj["geom"].toObject();
    auto p1 = QPointF(geom["x1"].toDouble(), geom["y1"].toDouble());
    auto p2 = QPointF(geom["x2"].toDouble(), geom["y2"].toDouble());
    auto s = std::make_unique<LineSegment>(p1, p2);
    s->FromJsonCommon(obj);
    return s;
}
