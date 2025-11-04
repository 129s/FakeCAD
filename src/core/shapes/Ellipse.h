#pragma once

#include <atomic>
#include <QPointF>
#include <QRectF>
#include "../Shape.h"

class Ellipse : public AreaShape {
public:
    Ellipse(const QPointF& c = {}, double rx = 0.0, double ry = 0.0);
    ~Ellipse() override;

    QString typeName() const override { return QStringLiteral("Ellipse"); }

    QRectF BoundingBox() const override {
        QRectF local(center_.x()-rx_, center_.y()-ry_, rx_*2, ry_*2);
        return transform().mapRect(local.normalized());
    }

    double Area() const override { return 3.14159265358979323846 * rx_ * ry_; }
    double Perimeter() const override; // Ramanujan approximation

    const QPointF& center() const { return center_; }
    void setCenter(const QPointF& c) { center_ = c; }
    double rx() const { return rx_; }
    double ry() const { return ry_; }
    void setRx(double v) { rx_ = std::max(0.0, v); }
    void setRy(double v) { ry_ = std::max(0.0, v); }

    QJsonObject ToJson() const override;
    static std::unique_ptr<Ellipse> FromJson(const QJsonObject& obj);

    static int Count() { return kCount.load(); }

private:
    QPointF center_{};
    double rx_{};
    double ry_{};
    inline static std::atomic<int> kCount{0};
};

