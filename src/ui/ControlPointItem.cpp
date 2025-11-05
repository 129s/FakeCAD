#include "ControlPointItem.h"

#include <QGraphicsSceneMouseEvent>
#include <QPen>
#include <QtMath>

#include "DrawingScene.h"
#include <QCursor>

#include "ShapeItem.h"
#include "../undo/Commands.h"
#include <QUndoStack>

ControlPointItem::ControlPointItem(ShapeItem* owner, Kind kind, int index, const QRectF& rect)
    : QGraphicsRectItem(rect), owner_(owner), kind_(kind), index_(index) {
    setParentItem(owner);
    setZValue(10'000); // on top
    setBrush(Qt::white);
    setPen(QPen(Qt::black));
    setFlag(ItemIsMovable, true);
    setFlag(ItemIgnoresTransformations, true);
    setCursor(QCursor(Qt::SizeAllCursor));
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
    } else {
        if (owner_ && owner_->model()) {
            oldJson_ = owner_->model()->ToJson();
        }
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
        owner_->model()->setRotationDegrees(owner_->rotation());
        owner_->updateHandles();
        return; // 不调用基类移动，以免把手柄位置改乱
    }
    // 非旋转手柄：将现场坐标映射到父项局部坐标（考虑吸附）
    QPointF sp = event->scenePos();
    if (owner_->scene()) {
        if (auto ds = dynamic_cast<class DrawingScene*>(owner_->scene())) {
            sp = ds->snapPoint(sp);
        }
    }
    const QPointF local = owner_->mapFromScene(sp);
    owner_->handleMoved(static_cast<ShapeItem::HandleKind>(kind_), index_, local, event->scenePos(), false);
    owner_->updateHandles();
}

void ControlPointItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    if (kind_ != Kind::Rotation) {
        QPointF sp = event->scenePos();
        if (owner_->scene()) {
            if (auto ds = dynamic_cast<class DrawingScene*>(owner_->scene())) {
                sp = ds->snapPoint(sp);
            }
        }
        const QPointF local = owner_->mapFromScene(sp);
        owner_->handleMoved(static_cast<ShapeItem::HandleKind>(kind_), index_, local, event->scenePos(), true);
        owner_->updateHandles();
        if (owner_ && owner_->model()) {
            QJsonObject neo = owner_->model()->ToJson();
            if (auto ds = dynamic_cast<class DrawingScene*>(owner_->scene())) {
                if (auto st = ds->undoStack()) {
                    st->push(new UndoCmd::EditShapeJsonCommand(owner_, oldJson_, neo));
                }
            }
        }
    } else {
        const qreal newRot = owner_->rotation();
        if (std::abs(newRot - initialOwnerRotation_) > 0.1) {
            if (auto ds = dynamic_cast<class DrawingScene*>(owner_->scene())) {
                if (auto st = ds->undoStack()) {
                    st->push(new UndoCmd::TransformShapeCommand(owner_, owner_->pos(), initialOwnerRotation_, owner_->pos(), newRot));
                }
            }
        }
    }
    QGraphicsRectItem::mouseReleaseEvent(event);
}
