#include "Rectangle.h"

Rectangle::Rectangle(const QRectF& r) : rect_(r) { ++kCount; }

Rectangle::~Rectangle() { --kCount; }

QJsonObject Rectangle::ToJson() const {
    QJsonObject obj = Shape::ToJson();
    obj["type"] = QStringLiteral("Rectangle");
    auto r = rect_.normalized();
    obj["geom"] = QJsonObject{{"x", r.x()}, {"y", r.y()}, {"w", r.width()}, {"h", r.height()}};
    return obj;
}

std::unique_ptr<Rectangle> Rectangle::FromJson(const QJsonObject& obj) {
    auto g = obj["geom"].toObject();
    QRectF r(g["x"].toDouble(), g["y"].toDouble(), g["w"].toDouble(), g["h"].toDouble());
    auto s = std::make_unique<Rectangle>(r);
    s->FromJsonCommon(obj);
    return s;
}
