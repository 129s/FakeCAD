#include "Circle.h"

Circle::Circle(const QPointF& c, double r) : center_(c), radius_(r) { ++kCount; }

Circle::~Circle() { --kCount; }

QJsonObject Circle::ToJson() const {
    QJsonObject obj = Shape::ToJson();
    obj["type"] = QStringLiteral("Circle");
    obj["geom"] = QJsonObject{{"cx", center_.x()}, {"cy", center_.y()}, {"r", radius_}};
    return obj;
}

std::unique_ptr<Circle> Circle::FromJson(const QJsonObject& obj) {
    auto g = obj["geom"].toObject();
    QPointF c(g["cx"].toDouble(), g["cy"].toDouble());
    double r = g["r"].toDouble();
    auto s = std::make_unique<Circle>(c, r);
    s->FromJsonCommon(obj);
    return s;
}
