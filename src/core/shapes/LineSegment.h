#pragma once

#include <atomic>
#include <QPointF>
#include <QRectF>
#include "../Shape.h"

class LineSegment : public LineShape {
public:
    LineSegment(const QPointF& p1 = {}, const QPointF& p2 = {});
    ~LineSegment() override;

    QString typeName() const override { return QStringLiteral("LineSegment"); }

    int VertexCount() const override { return 2; }
    QVector<QPointF> Vertices() const override { return {p1_, p2_}; }

    QRectF BoundingBox() const override;

    // 端点访问
    const QPointF& p1() const { return p1_; }
    const QPointF& p2() const { return p2_; }
    void setP1(const QPointF& p) { p1_ = p; }
    void setP2(const QPointF& p) { p2_ = p; }

    static int Count() { return kCount.load(); }

private:
    QPointF p1_;
    QPointF p2_;
    inline static std::atomic<int> kCount{0};
};

