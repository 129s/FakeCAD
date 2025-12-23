#pragma once

#include <memory>
#include <QGraphicsItem>
#include "../core/Shape.h"
#include "DrawingScene.h"
#include "../core/shapes/LineSegment.h"
#include "../core/shapes/Rectangle.h"
#include "../core/shapes/Circle.h"
#include "../core/shapes/Triangle.h"
#include "../core/shapes/Polygon.h"
#include "../core/shapes/Polyline.h"
#include "../core/shapes/Ellipse.h"

class ShapeItem : public QGraphicsItem {
    friend class ControlPointItem;
public:
    explicit ShapeItem(std::unique_ptr<Shape> shape, QGraphicsItem* parent = nullptr);
    ~ShapeItem() override = default;

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    Shape* model() const { return shape_.get(); }
    QString typeName() const { return shape_ ? shape_->typeName() : QString(); }
    // 控制点支持（公开以便外部刷新）
    void showHandles(bool show);
    void updateHandles();
    // 供外部（如撤销命令）使用：先通知几何即将变化，再完成变化并刷新
    void aboutToChangeGeometry() { prepareGeometryChange(); }
    void geometryChanged() { updateTransformOrigin(); update(); }
    // 控制点移动（提供给 ControlPointItem 调用）
    enum class HandleKind { Vertex, Corner, Center, Radius, Rotation };
    void handleMoved(HandleKind kind, int index, const QPointF& localPos, const QPointF& scenePos, bool release);
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override {
        if (change == ItemSelectedHasChanged) {
            bool sel = value.toBool();
            showHandles(sel);
        } else if (change == ItemRotationHasChanged) {
            if (!handlesFrozen_) updateHandles();
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


private:
    std::unique_ptr<Shape> shape_;
    QList<class QGraphicsItem*> handles_;
    class ControlPointItem* rotationHandle_ { nullptr };
    void clearHandles();
    void updateTransformOrigin();
    void setHandlesFrozen(bool on) { handlesFrozen_ = on; }
    void syncHandlesPositions(HandleKind activeKind, int activeIndex);

    // move tracking
    QPointF pressPos_{};
    double pressRot_{};
    bool moving_{false};

    bool handlesFrozen_{false};
};
