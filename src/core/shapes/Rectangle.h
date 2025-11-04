#pragma once

#include <atomic>
#include <QRectF>
#include "../Shape.h"

class Rectangle : public AreaShape {
public:
    explicit Rectangle(const QRectF& r = QRectF());
    ~Rectangle() override;

    QString typeName() const override { return QStringLiteral("Rectangle"); }

    QRectF BoundingBox() const override { return transform().mapRect(rect_.normalized()); }

    double Area() const override { return std::abs(rect_.width() * rect_.height()); }
    double Perimeter() const override { return 2.0 * (std::abs(rect_.width()) + std::abs(rect_.height())); }

    const QRectF& rect() const { return rect_; }
    void setRect(const QRectF& r) { rect_ = r; }

    QJsonObject ToJson() const override;
    static std::unique_ptr<Rectangle> FromJson(const QJsonObject& obj);

    static int Count() { return kCount.load(); }

private:
    QRectF rect_;
    inline static std::atomic<int> kCount{0};
};
