#pragma once

#include <memory>
#include <QGraphicsItem>
#include "../core/Shape.h"
#include "DrawingScene.h"
#include "../core/shapes/LineSegment.h"
#include "../core/shapes/Rectangle.h"
#include "../core/shapes/Circle.h"

class ShapeItem : public QGraphicsItem {
public:
    explicit ShapeItem(std::unique_ptr<Shape> shape, QGraphicsItem* parent = nullptr);
    ~ShapeItem() override = default;

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override {
        if (change == ItemSelectedHasChanged) {
            bool sel = value.toBool();
            showHandles(sel);
        } else if (change == ItemRotationHasChanged) {
            updateHandles();
        } else if (change == ItemPositionChange) {
            // 位置吸附到网格
            if (scene()) {
                if (auto ds = dynamic_cast<class DrawingScene*>(scene())) {
                    if (ds->snapToGrid()) {
                        QPointF p = value.toPointF();
                        const qreal s = ds->gridSize();
                        p.setX(std::round(p.x()/s)*s);
                        p.setY(std::round(p.y()/s)*s);
                        return p;
                    }
                }
            }
        }
        return QGraphicsItem::itemChange(change, value);
    }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

    Shape* shape() const { return shape_.get(); }
    QString typeName() const { return shape_ ? shape_->typeName() : QString(); }

    // 控制点支持
    void showHandles(bool show);
    void updateHandles();

    // 内部回调：控制点移动
    enum class HandleKind { Vertex, Corner, Center, Radius, Rotation };
    void handleMoved(HandleKind kind, int index, const QPointF& localPos, const QPointF& scenePos, bool release);

private:
    std::unique_ptr<Shape> shape_;
    QList<class QGraphicsItem*> handles_;
    class ControlPointItem* rotationHandle_ { nullptr };
    void clearHandles();

    // move tracking
    QPointF pressPos_{};
    double pressRot_{};
    bool moving_{false};
};
