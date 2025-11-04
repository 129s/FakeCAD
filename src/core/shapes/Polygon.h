#pragma once

#include <atomic>
#include <QVector>
#include <QPointF>
#include <QRectF>
#include "../Shape.h"

class Polygon : public AreaShape {
public:
    Polygon() { ++kCount; }
    explicit Polygon(const QVector<QPointF>& pts) : points_(pts) { ++kCount; }
    ~Polygon() override { --kCount; }

    QString typeName() const override { return QStringLiteral("Polygon"); }

    QRectF BoundingBox() const override { return transform().mapRect(QPolygonF(points_).boundingRect()); }

    double Area() const override;
    double Perimeter() const override;

    const QVector<QPointF>& points() const { return points_; }
    void setPoints(const QVector<QPointF>& pts) { points_ = pts; }
    void setPoint(int i, const QPointF& p) { if (i>=0 && i<points_.size()) points_[i]=p; }

    QJsonObject ToJson() const override;
    static std::unique_ptr<Polygon> FromJson(const QJsonObject& obj);

    static int Count() { return kCount.load(); }

private:
    QVector<QPointF> points_{};
    inline static std::atomic<int> kCount{0};
};

