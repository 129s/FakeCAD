#pragma once

#include <QGraphicsRectItem>
#include <QPointF>

class ShapeItem;

class ControlPointItem : public QGraphicsRectItem {
public:
    enum class Kind { Vertex, Corner, Center, Radius, Rotation };

    ControlPointItem(ShapeItem* owner, Kind kind, int index, const QRectF& rect);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    ShapeItem* owner_ { nullptr };
    Kind kind_ { Kind::Vertex };
    int index_ { 0 };

    // for rotation
    QPointF pressScenePos_ {};
    QPointF centerScene_ {};
    qreal initialOwnerRotation_ { 0.0 };
};

