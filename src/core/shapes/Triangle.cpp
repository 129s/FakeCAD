#include "Triangle.h"

#include <cmath>

Triangle::Triangle(const QPointF& a, const QPointF& b, const QPointF& c)
    : a_(a), b_(b), c_(c) { ++kCount; }

Triangle::~Triangle() { --kCount; }

QRectF Triangle::BoundingBox() const {
    QPolygonF poly; poly << a_ << b_ << c_;
    return transform().mapRect(poly.boundingRect());
}

double Triangle::Area() const {
    // Shoelace for triangle
    return std::abs(a_.x()*b_.y() + b_.x()*c_.y() + c_.x()*a_.y() - a_.y()*b_.x() - b_.y()*c_.x() - c_.y()*a_.x()) * 0.5;
}

double Triangle::Perimeter() const {
    auto d = [](const QPointF& u, const QPointF& v){ return std::hypot(u.x()-v.x(), u.y()-v.y()); };
    return d(a_, b_) + d(b_, c_) + d(c_, a_);
}

QJsonObject Triangle::ToJson() const {
    QJsonObject obj = Shape::ToJson();
    obj["type"] = QStringLiteral("Triangle");
    obj["geom"] = QJsonObject{{"x1", a_.x()}, {"y1", a_.y()}, {"x2", b_.x()}, {"y2", b_.y()}, {"x3", c_.x()}, {"y3", c_.y()}};
    return obj;
}

std::unique_ptr<Triangle> Triangle::FromJson(const QJsonObject& obj) {
    auto g = obj["geom"].toObject();
    QPointF a(g["x1"].toDouble(), g["y1"].toDouble());
    QPointF b(g["x2"].toDouble(), g["y2"].toDouble());
    QPointF c(g["x3"].toDouble(), g["y3"].toDouble());
    auto s = std::make_unique<Triangle>(a, b, c);
    s->FromJsonCommon(obj);
    return s;
}

