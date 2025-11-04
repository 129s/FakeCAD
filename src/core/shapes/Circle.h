#pragma once

#include <atomic>
#include <QPointF>
#include <QRectF>
#include "../Shape.h"
#include <algorithm>

class Circle : public AreaShape {
public:
    Circle(const QPointF& c = {}, double r = 0.0);
    ~Circle() override;

    QString typeName() const override { return QStringLiteral("Circle"); }

    QRectF BoundingBox() const override {
        QRectF local(center_.x() - radius_, center_.y() - radius_, radius_ * 2.0, radius_ * 2.0);
        return transform().mapRect(local.normalized());
    }

    double Area() const override { return 3.14159265358979323846 * radius_ * radius_; }
    double Perimeter() const override { return 2.0 * 3.14159265358979323846 * radius_; }

    const QPointF& center() const { return center_; }
    double radius() const { return radius_; }
    void setCenter(const QPointF& c) { center_ = c; }
    void setRadius(double r) { radius_ = std::max(0.0, r); }

    QJsonObject ToJson() const override;
    static std::unique_ptr<Circle> FromJson(const QJsonObject& obj);

    static int Count() { return kCount.load(); }

private:
    QPointF center_{};
    double radius_{};
    inline static std::atomic<int> kCount{0};
};
