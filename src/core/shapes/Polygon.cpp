#include "Polygon.h"

#include <cmath>

static double poly_perimeter(const QVector<QPointF>& pts) {
    if (pts.size() < 2) return 0.0;
    double sum = 0.0;
    for (int i = 0; i < pts.size(); ++i) {
        const auto& a = pts[i];
        const auto& b = pts[(i + 1) % pts.size()];
        sum += std::hypot(b.x() - a.x(), b.y() - a.y());
    }
    return sum;
}

static double poly_area(const QVector<QPointF>& pts) {
    if (pts.size() < 3) return 0.0;
    double sum = 0.0;
    for (int i = 0; i < pts.size(); ++i) {
        const auto& a = pts[i];
        const auto& b = pts[(i + 1) % pts.size()];
        sum += a.x() * b.y() - b.x() * a.y();
    }
    return std::abs(sum) * 0.5;
}

double Polygon::Area() const { return poly_area(points_); }
double Polygon::Perimeter() const { return poly_perimeter(points_); }

QJsonObject Polygon::ToJson() const {
    QJsonObject obj = Shape::ToJson();
    obj["type"] = QStringLiteral("Polygon");
    QJsonArray arr;
    for (const auto& p : points_) arr.append(QJsonObject{{"x", p.x()}, {"y", p.y()}});
    obj["geom"] = QJsonObject{{"points", arr}};
    return obj;
}

std::unique_ptr<Polygon> Polygon::FromJson(const QJsonObject& obj) {
    auto g = obj["geom"].toObject();
    auto arr = g["points"].toArray();
    QVector<QPointF> pts; pts.reserve(arr.size());
    for (const auto& v : arr) {
        auto o = v.toObject();
        pts.append(QPointF(o["x"].toDouble(), o["y"].toDouble()));
    }
    auto s = std::make_unique<Polygon>(pts);
    s->FromJsonCommon(obj);
    return s;
}

