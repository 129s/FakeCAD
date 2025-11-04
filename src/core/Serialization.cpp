#include "Serialization.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QFile>

#include "shapes/LineSegment.h"
#include "shapes/Rectangle.h"
#include "shapes/Circle.h"

namespace Ser {

static QJsonObject shapeToJson(const Shape& s) {
    // 使用虚函数 ToJson 生成对象（其中包含 type 与 geom）
    return s.ToJson();
}

QJsonDocument Serialize(const std::vector<Shape*>& shapes) {
    QJsonArray arr;
    for (auto* s : shapes) {
        if (!s) continue;
        arr.append(shapeToJson(*s));
    }
    QJsonObject root{{"version", 1}, {"shapes", arr}};
    return QJsonDocument(root);
}

std::unique_ptr<Shape> FromJsonObject(const QJsonObject& obj) {
    const auto type = obj["type"].toString();
    if (type == QStringLiteral("LineSegment")) return LineSegment::FromJson(obj);
    if (type == QStringLiteral("Rectangle"))   return Rectangle::FromJson(obj);
    if (type == QStringLiteral("Circle"))      return Circle::FromJson(obj);
    return {};
}

std::vector<std::unique_ptr<Shape>> Deserialize(const QJsonDocument& doc) {
    std::vector<std::unique_ptr<Shape>> out;
    if (!doc.isObject()) return out;
    auto root = doc.object();
    auto arr = root["shapes"].toArray();
    out.reserve(arr.size());
    for (const auto& v : arr) {
        auto obj = v.toObject();
        if (auto s = FromJsonObject(obj)) out.push_back(std::move(s));
    }
    return out;
}

bool SaveToFile(const QString& path, const std::vector<Shape*>& shapes, QString* error) {
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly)) {
        if (error) *error = f.errorString();
        return false;
    }
    auto doc = Serialize(shapes);
    f.write(doc.toJson(QJsonDocument::Indented));
    return true;
}

std::vector<std::unique_ptr<Shape>> LoadFromFile(const QString& path, QString* error) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        if (error) *error = f.errorString();
        return {};
    }
    auto data = f.readAll();
    QJsonParseError perr{};
    auto doc = QJsonDocument::fromJson(data, &perr);
    if (perr.error != QJsonParseError::NoError) {
        if (error) *error = perr.errorString();
        return {};
    }
    return Deserialize(doc);
}

bool ApplyJsonToShape(Shape* s, const QJsonObject& obj) {
    if (!s) return false;
    if (obj["type"].toString() != s->typeName()) return false;
    s->FromJsonCommon(obj);
    auto g = obj["geom"].toObject();
    if (auto* ls = dynamic_cast<LineSegment*>(s)) {
        QPointF p1(g["x1"].toDouble(), g["y1"].toDouble());
        QPointF p2(g["x2"].toDouble(), g["y2"].toDouble());
        ls->setP1(p1); ls->setP2(p2);
        return true;
    }
    if (auto* rc = dynamic_cast<Rectangle*>(s)) {
        QRectF r(g["x"].toDouble(), g["y"].toDouble(), g["w"].toDouble(), g["h"].toDouble());
        rc->setRect(r);
        return true;
    }
    if (auto* cc = dynamic_cast<Circle*>(s)) {
        QPointF c(g["cx"].toDouble(), g["cy"].toDouble());
        double r = g["r"].toDouble();
        cc->setCenter(c); cc->setRadius(r);
        return true;
    }
    return false;
}

} // namespace Ser
