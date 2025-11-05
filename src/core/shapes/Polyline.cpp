#include "Polyline.h"
#include <QJsonArray>

QJsonObject Polyline::ToJson() const {
    QJsonObject obj = Shape::ToJson();
    obj["type"] = QStringLiteral("Polyline");
    QJsonArray arr;
    for (const auto& p : points_) arr.append(QJsonObject{{"x", p.x()}, {"y", p.y()}});
    obj["geom"] = QJsonObject{{"points", arr}};
    return obj;
}

std::unique_ptr<Polyline> Polyline::FromJson(const QJsonObject& obj) {
    auto g = obj["geom"].toObject();
    auto arr = g["points"].toArray();
    QVector<QPointF> pts; pts.reserve(arr.size());
    for (const auto& v : arr) {
        auto o = v.toObject();
        pts.append(QPointF(o["x"].toDouble(), o["y"].toDouble()));
    }
    auto s = std::make_unique<Polyline>(pts);
    s->FromJsonCommon(obj);
    return s;
}
