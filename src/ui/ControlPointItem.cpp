#include "ControlPointItem.h"

#include <QGraphicsSceneMouseEvent>
#include <QPen>
#include <QtMath>
#include <QPainter>
#include <QPainterPath>
#include <cmath>

#include "DrawingScene.h"
#include <QCursor>
#include <QTimer>

#include "ShapeItem.h"
#include "../undo/Commands.h"
#include <QUndoStack>

ControlPointItem::ControlPointItem(ShapeItem* owner, Kind kind, int index, const QRectF& rect)
    : QGraphicsRectItem(rect), owner_(owner), kind_(kind), index_(index) {
    setParentItem(owner);
    setZValue(10'000); // on top
    setBrush(Qt::white);
    setPen(QPen(Qt::black));
    // 自行处理拖拽，避免 QGraphicsView::RubberBandDrag 等模式抢事件/与默认移动逻辑冲突
    setFlag(ItemIsMovable, false);
    setFlag(ItemIgnoresTransformations, true);
    setAcceptedMouseButtons(Qt::LeftButton);
    setCursor(QCursor(Qt::SizeAllCursor));
}

void ControlPointItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() != Qt::LeftButton) { event->ignore(); return; }
    event->accept();
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
    event->accept();
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
        // 让旋转手柄跟随鼠标（视觉反馈），松手后会被 updateHandles 复位
        if (owner_) setPos(owner_->mapFromScene(event->scenePos()));
        return;
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
    event->accept();
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
                    // 不能在控制点自身的鼠标事件回调里同步 push：
                    // QUndoStack::push() 会立即 redo()，命令里会 updateHandles() 并删除当前控制点，
                    // 造成“delete this”式的概率崩溃。延后一拍让事件先返回。
                    const auto oldJ = oldJson_;
                    QTimer::singleShot(0, st, [st, owner = owner_, oldJ, neo]() {
                        st->push(new UndoCmd::EditShapeJsonCommand(owner, oldJ, neo));
                    });
                }
            }
        }
        if (owner_) {
            // 拖拽过程中已实时同步控制点，松手不再重建（避免删除当前对象导致随机崩溃）
            owner_->syncHandlesPositions(static_cast<ShapeItem::HandleKind>(kind_), index_);
            return;
        }
    } else {
        const qreal newRot = owner_->rotation();
        if (std::abs(newRot - initialOwnerRotation_) > 0.1) {
            if (auto ds = dynamic_cast<class DrawingScene*>(owner_->scene())) {
                if (auto st = ds->undoStack()) {
                    // 同上：避免在旋转控制点自身的回调里同步 push 导致控制点被删除
                    const QPointF pos = owner_->pos();
                    const double oldRot = initialOwnerRotation_;
                    const double neoRot = newRot;
                    QTimer::singleShot(0, st, [st, owner = owner_, pos, oldRot, neoRot]() {
                        st->push(new UndoCmd::TransformShapeCommand(owner, pos, oldRot, pos, neoRot));
                    });
                }
            }
        }
        if (owner_) {
            owner_->setHandlesFrozen(false);
            // 旋转结束后把旋转手柄吸回到“包围盒顶部中心”
            owner_->syncHandlesPositions(ShapeItem::HandleKind::Vertex, -1);
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
