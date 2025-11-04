#include "ShapeItem.h"

#include <QPainter>
#include <algorithm>

#include "ControlPointItem.h"
#include <QGraphicsSceneMouseEvent>
#include "../undo/Commands.h"

ShapeItem::ShapeItem(std::unique_ptr<Shape> shape, QGraphicsItem* parent)
    : QGraphicsItem(parent), shape_(std::move(shape)) {
    setFlag(ItemIsSelectable, true);
    setFlag(ItemIsMovable, true);
    setFlag(ItemSendsGeometryChanges, true);
    setAcceptHoverEvents(true);
    // 使用模型中的平移与旋转初始化项位置
    const auto& t = shape_->transform();
    setPos(t.m31(), t.m32());
    setRotation(shape_->rotationDegrees());
}

QRectF ShapeItem::boundingRect() const {
    if (auto* ls = dynamic_cast<LineSegment*>(shape_.get())) {
        return QRectF(ls->p1(), ls->p2()).normalized().adjusted(-1, -1, 1, 1);
    }
    if (auto* rc = dynamic_cast<Rectangle*>(shape_.get())) {
        return rc->rect().normalized().adjusted(-1, -1, 1, 1);
    }
    if (auto* cc = dynamic_cast<Circle*>(shape_.get())) {
        const auto r = cc->radius();
        return QRectF(cc->center().x() - r, cc->center().y() - r, 2 * r, 2 * r).adjusted(-1, -1, 1, 1);
    }
    return {};
}

void ShapeItem::clearHandles() {
    for (auto* h : handles_) { if (h) { h->setParentItem(nullptr); scene()->removeItem(h); delete h; } }
    handles_.clear();
    if (rotationHandle_) { rotationHandle_->setParentItem(nullptr); scene()->removeItem(rotationHandle_); delete rotationHandle_; rotationHandle_ = nullptr; }
}

void ShapeItem::showHandles(bool show) {
    if (!show) { clearHandles(); return; }
    updateHandles();
}

void ShapeItem::updateHandles() {
    clearHandles();
    const qreal s = 8.0;
    auto mk = [&](HandleKind kind, int idx, const QPointF& pos){
        auto* h = new ControlPointItem(this, static_cast<ControlPointItem::Kind>(kind), idx, QRectF(-s/2, -s/2, s, s));
        h->setPos(pos);
        handles_.push_back(h);
        return h;
    };

    if (auto* ls = dynamic_cast<LineSegment*>(shape_.get())) {
        mk(HandleKind::Vertex, 0, ls->p1());
        mk(HandleKind::Vertex, 1, ls->p2());
    } else if (auto* rc = dynamic_cast<Rectangle*>(shape_.get())) {
        const auto r = rc->rect().normalized();
        mk(HandleKind::Corner, 0, r.topLeft());
        mk(HandleKind::Corner, 1, r.topRight());
        mk(HandleKind::Corner, 2, r.bottomRight());
        mk(HandleKind::Corner, 3, r.bottomLeft());
    } else if (auto* cc = dynamic_cast<Circle*>(shape_.get())) {
        const auto c = cc->center();
        mk(HandleKind::Center, 0, c);
        mk(HandleKind::Radius, 0, c + QPointF(cc->radius(), 0));
    }

    // 旋转手柄：放在局部包围盒顶部中心上方 30px
    const auto br = boundingRect();
    QPointF topCenter = QPointF((br.left()+br.right())/2.0, br.top());
    rotationHandle_ = new ControlPointItem(this, ControlPointItem::Kind::Rotation, 0, QRectF(-s/2, -s/2, s, s));
    rotationHandle_->setPos(topCenter + QPointF(0, -30));
}

void ShapeItem::handleMoved(HandleKind kind, int index, const QPointF& localPos, const QPointF& /*scenePos*/, bool /*release*/) {
    if (auto* ls = dynamic_cast<LineSegment*>(shape_.get())) {
        if (kind == HandleKind::Vertex) {
            if (index == 0) ls->setP1(localPos); else ls->setP2(localPos);
            prepareGeometryChange();
            update();
        }
    } else if (auto* rc = dynamic_cast<Rectangle*>(shape_.get())) {
        if (kind == HandleKind::Corner) {
            auto r = rc->rect();
            QPointF tl = r.topLeft();
            QPointF tr = r.topRight();
            QPointF br = r.bottomRight();
            QPointF bl = r.bottomLeft();
            switch (index) {
            case 0: tl = localPos; break;
            case 1: tr = localPos; break;
            case 2: br = localPos; break;
            case 3: bl = localPos; break;
            }
            QRectF nr(QPointF(std::min({tl.x(), tr.x(), br.x(), bl.x()}), std::min({tl.y(), tr.y(), br.y(), bl.y()})),
                      QPointF(std::max({tl.x(), tr.x(), br.x(), bl.x()}), std::max({tl.y(), tr.y(), br.y(), bl.y()})));
            rc->setRect(nr);
            prepareGeometryChange();
            update();
        }
    } else if (auto* cc = dynamic_cast<Circle*>(shape_.get())) {
        if (kind == HandleKind::Center) {
            cc->setCenter(localPos);
            prepareGeometryChange();
            update();
        } else if (kind == HandleKind::Radius) {
            const auto c = cc->center();
            const double r = std::hypot(localPos.x() - c.x(), localPos.y() - c.y());
            cc->setRadius(r);
            prepareGeometryChange();
            update();
        }
    }
}

void ShapeItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        pressPos_ = pos();
        pressRot_ = rotation();
        moving_ = true;
    }
    QGraphicsItem::mousePressEvent(event);
}

void ShapeItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    QGraphicsItem::mouseReleaseEvent(event);
    if (moving_ && event->button() == Qt::LeftButton) {
        moving_ = false;
        QPointF newPos = pos();
        double newRot = rotation();
        if ((std::hypot(newPos.x()-pressPos_.x(), newPos.y()-pressPos_.y()) > 0.1) || std::abs(newRot - pressRot_) > 0.1) {
            if (scene()) {
                if (auto ds = dynamic_cast<DrawingScene*>(scene())) {
                    if (auto st = ds->undoStack()) {
                        st->push(new UndoCmd::TransformShapeCommand(this, pressPos_, pressRot_, newPos, newRot));
                    }
                }
            }
        }
    }
}

void ShapeItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(shape_->pen());

    if (auto* ls = dynamic_cast<LineSegment*>(shape_.get())) {
        painter->drawLine(ls->p1(), ls->p2());
        const double L = ls->Length();
        painter->setPen(QPen(Qt::darkGray));
        painter->drawText(boundingRect().translated(4, -4).topLeft(), QString("L=%1").arg(L, 0, 'f', 2));
        return;
    }
    if (auto* rc = dynamic_cast<Rectangle*>(shape_.get())) {
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(rc->rect());
        painter->setPen(QPen(Qt::darkGray));
        const double P = rc->Perimeter();
        const double A = rc->Area();
        painter->drawText(rc->rect().topLeft() + QPointF(4, 12), QString("P=%1 A=%2").arg(P, 0, 'f', 2).arg(A, 0, 'f', 2));
        return;
    }
    if (auto* cc = dynamic_cast<Circle*>(shape_.get())) {
        painter->setBrush(Qt::NoBrush);
        const auto r = cc->radius();
        painter->drawEllipse(cc->center(), r, r);
        painter->setPen(QPen(Qt::darkGray));
        const double P = cc->Perimeter();
        const double A = cc->Area();
        painter->drawText(boundingRect().translated(4, -4).topLeft(), QString("P=%1 A=%2").arg(P, 0, 'f', 2).arg(A, 0, 'f', 2));
        return;
    }
}
