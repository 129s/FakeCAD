#pragma once

#include <atomic>
#include <QPointF>
#include <QRectF>
#include "../Shape.h"

class Triangle : public AreaShape {
public:
    Triangle(const QPointF& a = {}, const QPointF& b = {}, const QPointF& c = {});
    ~Triangle() override;

    QString typeName() const override { return QStringLiteral("Triangle"); }

    QRectF BoundingBox() const override;
    double Area() const override;
    double Perimeter() const override;

    const QPointF& p1() const { return a_; }
    const QPointF& p2() const { return b_; }
    const QPointF& p3() const { return c_; }
    void setP1(const QPointF& p) { a_ = p; }
    void setP2(const QPointF& p) { b_ = p; }
    void setP3(const QPointF& p) { c_ = p; }

    QJsonObject ToJson() const override;
    static std::unique_ptr<Triangle> FromJson(const QJsonObject& obj);

    static int Count() { return kCount.load(); }

private:
    QPointF a_, b_, c_;
    inline static std::atomic<int> kCount{0};
};

