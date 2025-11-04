#include "Shape.h"

#include <QtMath>
#include <cmath>

QJsonObject Shape::ToJson() const {
    QJsonObject obj;
    obj["name"] = name_;
    obj["style"] = QJsonObject{
        {"color", color_.name(QColor::HexArgb)},
        {"pen", QJsonObject{{"width", pen_.widthF()}}}
    };
    // 简化的变换，仅保存平移；旋转/缩放后续扩展
    obj["transform"] = QJsonObject{{"tx", transform_.m31()}, {"ty", transform_.m32()}};
    return obj;
}

void Shape::FromJsonCommon(const QJsonObject& obj) {
    if (obj.contains("name")) name_ = obj["name"].toString();
    if (obj.contains("style")) {
        auto s = obj["style"].toObject();
        if (s.contains("color")) color_.setNamedColor(s["color"].toString());
        if (s.contains("pen")) {
            auto p = s["pen"].toObject();
            if (p.contains("width")) pen_.setWidthF(p["width"].toDouble());
        }
    }
    if (obj.contains("transform")) {
        auto t = obj["transform"].toObject();
        auto tx = t["tx"].toDouble();
        auto ty = t["ty"].toDouble();
        QTransform newT = transform_;
        newT.setMatrix(newT.m11(), newT.m12(), newT.m13(),
                       newT.m21(), newT.m22(), newT.m23(),
                       tx,          ty,          newT.m33());
        transform_ = newT;
    }
}

double LineShape::Length() const {
    const auto pts = Vertices();
    if (pts.size() < 2) return 0.0;
    double sum = 0.0;
    for (int i = 1; i < pts.size(); ++i) {
        const auto dx = pts[i].x() - pts[i - 1].x();
        const auto dy = pts[i].y() - pts[i - 1].y();
        sum += std::hypot(dx, dy);
    }
    return sum;
}
