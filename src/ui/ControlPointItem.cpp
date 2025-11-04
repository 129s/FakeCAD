#include "ControlPointItem.h"

#include <QGraphicsSceneMouseEvent>
#include <QPen>
#include <QtMath>

#include "ShapeItem.h"

ControlPointItem::ControlPointItem(ShapeItem* owner, Kind kind, int index, const QRectF& rect)
    : QGraphicsRectItem(rect), owner_(owner), kind_(kind), index_(index) {
    setParentItem(owner);
    setZValue(10'000); // on top
    setBrush(Qt::white);
    setPen(QPen(Qt::black));
    setFlag(ItemIsMovable, true);
    setFlag(ItemIgnoresTransformations, true);
    setCursor(Qt::SizeAllCursor);
}

void ControlPointItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    QGraphicsRectItem::mousePressEvent(event);
    pressScenePos_ = event->scenePos();
    // rotation support
    if (kind_ == Kind::Rotation) {
        initialOwnerRotation_ = owner_->rotation();
        // use owner's local bounding rect center as pivot
        QPointF centerLocal = owner_->boundingRect().center();
        centerScene_ = owner_->mapToScene(centerLocal);
    }
}

void ControlPointItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    if (kind_ == Kind::Rotation) {
        // compute delta angle relative to pivot
        QPointF v0 = pressScenePos_ - centerScene_;
        QPointF v1 = event->scenePos() - centerScene_;
        if (qFuzzyIsNull(v0.manhattanLength()) || qFuzzyIsNull(v1.manhattanLength())) return;
        qreal a0 = qRadiansToDegrees(std::atan2(v0.y(), v0.x()));
        qreal a1 = qRadiansToDegrees(std::atan2(v1.y(), v1.x()));
        qreal delta = a1 - a0;
        owner_->setRotation(initialOwnerRotation_ + delta);
        // 同步模型角度
        owner_->shape()->setRotationDegrees(owner_->rotation());
        owner_->updateHandles();
        return; // 不调用基类移动，以免把手柄位置改乱
    }
    // 非旋转手柄：将现场坐标映射到父项局部坐标
    const QPointF local = owner_->mapFromScene(event->scenePos());
    owner_->handleMoved(static_cast<ShapeItem::HandleKind>(kind_), index_, local, event->scenePos(), false);
    owner_->updateHandles();
}

void ControlPointItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    if (kind_ != Kind::Rotation) {
        const QPointF local = owner_->mapFromScene(event->scenePos());
        owner_->handleMoved(static_cast<ShapeItem::HandleKind>(kind_), index_, local, event->scenePos(), true);
        owner_->updateHandles();
    }
    QGraphicsRectItem::mouseReleaseEvent(event);
}

