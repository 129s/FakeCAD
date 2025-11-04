#include "Ellipse.h"

#include <cmath>

Ellipse::Ellipse(const QPointF& c, double rx, double ry)
    : center_(c), rx_(rx), ry_(ry) { ++kCount; }

Ellipse::~Ellipse() { --kCount; }

double Ellipse::Perimeter() const {
    // Ramanujan approximation: pi [3(a+b) - sqrt{(3a+b)(a+3b)}]
    const double a = std::max(rx_, ry_);
    const double b = std::min(rx_, ry_);
    if (a <= 0.0 || b <= 0.0) return 0.0;
    const double h = (3*(a+b) - std::sqrt((3*a + b)*(a + 3*b)));
    return 3.14159265358979323846 * h;
}

QJsonObject Ellipse::ToJson() const {
    QJsonObject obj = Shape::ToJson();
    obj["type"] = QStringLiteral("Ellipse");
    obj["geom"] = QJsonObject{{"cx", center_.x()}, {"cy", center_.y()}, {"rx", rx_}, {"ry", ry_}};
    return obj;
}

std::unique_ptr<Ellipse> Ellipse::FromJson(const QJsonObject& obj) {
    auto g = obj["geom"].toObject();
    QPointF c(g["cx"].toDouble(), g["cy"].toDouble());
    double rx = g["rx"].toDouble();
    double ry = g["ry"].toDouble();
    auto s = std::make_unique<Ellipse>(c, rx, ry);
    s->FromJsonCommon(obj);
    return s;
}

