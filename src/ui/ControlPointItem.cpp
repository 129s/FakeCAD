#include "ControlPointItem.h"

#include <QGraphicsSceneMouseEvent>
#include <QPen>
#include <QtMath>
#include <QPainter>
#include <QPainterPath>
#include <cmath>

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
        if (!owner_) return;
        initialOwnerRotation_ = owner_->rotation();
        // use owner's local bounding rect center as pivot
        QPointF centerLocal = owner_->boundingRect().center();
        centerScene_ = owner_->mapToScene(centerLocal);
        owner_->setHandlesFrozen(true);
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
    setPos(local);
}

void ControlPointItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    QGraphicsRectItem::mouseReleaseEvent(event);
    if (kind_ != Kind::Rotation) {
        QPointF sp = event->scenePos();
        if (owner_->scene()) {
            if (auto ds = dynamic_cast<class DrawingScene*>(owner_->scene())) {
                sp = ds->snapPoint(sp);
            }
        }
        const QPointF local = owner_->mapFromScene(sp);
        owner_->handleMoved(static_cast<ShapeItem::HandleKind>(kind_), index_, local, event->scenePos(), true);
        if (owner_ && owner_->model()) {
            QJsonObject neo = owner_->model()->ToJson();
            if (auto ds = dynamic_cast<class DrawingScene*>(owner_->scene())) {
                if (auto st = ds->undoStack()) {
                    st->push(new UndoCmd::EditShapeJsonCommand(owner_, oldJson_, neo));
                }
            }
        }
        if (owner_) {
            owner_->updateHandles();
            return; // 当前手柄已被 updateHandles 删除
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
        if (owner_) {
            owner_->setHandlesFrozen(false);
            owner_->updateHandles();
        }
        return; // rotation handle deleted during updateHandles()
    }
}

void ControlPointItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    if (kind_ != Kind::Rotation) {
        QGraphicsRectItem::paint(painter, option, widget);
        return;
    }
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    const QRectF r = rect();
    QPen outline(Qt::black);
    outline.setWidthF(1.0);
    painter->setPen(outline);
    painter->setBrush(Qt::white);
    painter->drawEllipse(r);

    QPen arcPen(Qt::black);
    arcPen.setWidthF(1.4);
    painter->setPen(arcPen);
    const QRectF arcRect = r.adjusted(2, 2, -2, -2);
    QPainterPath arc;
    const qreal startDeg = -45.0;
    const qreal spanDeg = -270.0;
    arc.arcMoveTo(arcRect, startDeg);
    arc.arcTo(arcRect, startDeg, spanDeg);
    painter->drawPath(arc);

    const qreal endDeg = startDeg + spanDeg;
    const qreal endRad = qDegreesToRadians(endDeg);
    const QPointF center = r.center();
    const qreal rx = arcRect.width() / 2.0;
    const qreal ry = arcRect.height() / 2.0;
    QPointF tip(center.x() + rx * std::cos(endRad),
                center.y() - ry * std::sin(endRad));

    auto scaleVec = [](const QPointF& v, qreal s) { return QPointF(v.x() * s, v.y() * s); };
    auto length = [](const QPointF& v) { return std::hypot(v.x(), v.y()); };

    QPointF radiusVec = tip - center;
    qreal radiusLen = length(radiusVec);
    QPointF radiusUnit = radiusLen > 1e-3 ? scaleVec(radiusVec, 1.0 / radiusLen) : QPointF();
    QPointF tangent(-radiusUnit.y(), radiusUnit.x());

    const qreal headLen = 4.0;
    const qreal headSpread = 2.0;
    QPointF left = tip - scaleVec(tangent, headLen) + scaleVec(radiusUnit, headSpread);
    QPointF right = tip - scaleVec(tangent, headLen) - scaleVec(radiusUnit, headSpread);

    QPainterPath head;
    head.moveTo(tip);
    head.lineTo(left);
    head.lineTo(right);
    head.closeSubpath();
    painter->fillPath(head, arcPen.color());

    painter->restore();
}
