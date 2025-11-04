#pragma once

#include <atomic>
#include <QVector>
#include <QPointF>
#include <QRectF>
#include "../Shape.h"

class Polyline : public LineShape {
public:
    Polyline() { ++kCount; }
    explicit Polyline(const QVector<QPointF>& pts) : points_(pts) { ++kCount; }
    ~Polyline() override { --kCount; }

    QString typeName() const override { return QStringLiteral("Polyline"); }

    int VertexCount() const override { return points_.size(); }
    QVector<QPointF> Vertices() const override { return points_; }

    QRectF BoundingBox() const override { return transform().mapRect(QPolygonF(points_).boundingRect()); }

    const QVector<QPointF>& points() const { return points_; }
    void setPoints(const QVector<QPointF>& pts) { points_ = pts; }
    void setPoint(int i, const QPointF& p) { if (i>=0 && i<points_.size()) points_[i]=p; }

    QJsonObject ToJson() const override;
    static std::unique_ptr<Polyline> FromJson(const QJsonObject& obj);

    static int Count() { return kCount.load(); }

private:
    QVector<QPointF> points_{};
    inline static std::atomic<int> kCount{0};
};

