#include "ShapeItem.h"

#include <QPainter>
#include <algorithm>
#include <cmath>

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
    updateTransformOrigin();
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
    if (auto* tr = dynamic_cast<Triangle*>(shape_.get())) {
        QPolygonF poly; poly << tr->p1() << tr->p2() << tr->p3();
        return poly.boundingRect().adjusted(-1, -1, 1, 1);
    }
    if (auto* pg = dynamic_cast<Polygon*>(shape_.get())) {
        return QPolygonF(pg->points()).boundingRect().adjusted(-1, -1, 1, 1);
    }
    if (auto* pl = dynamic_cast<Polyline*>(shape_.get())) {
        return QPolygonF(pl->points()).boundingRect().adjusted(-1, -1, 1, 1);
    }
    if (auto* el = dynamic_cast<Ellipse*>(shape_.get())) {
        return QRectF(el->center().x()-el->rx(), el->center().y()-el->ry(), el->rx()*2, el->ry()*2).adjusted(-1,-1,1,1);
    }
    return {};
}

void ShapeItem::clearHandles() {
    auto* sc = scene();
    for (auto* h : handles_) { if (h) { h->setParentItem(nullptr); if (sc) sc->removeItem(h); delete h; } }
    handles_.clear();
    if (rotationHandle_) {
        rotationHandle_->setParentItem(nullptr);
        if (sc) sc->removeItem(rotationHandle_);
        delete rotationHandle_;
        rotationHandle_ = nullptr;
    }
}

void ShapeItem::updateTransformOrigin() {
    setTransformOriginPoint(boundingRect().center());
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
    } else if (auto* tr = dynamic_cast<Triangle*>(shape_.get())) {
        mk(HandleKind::Vertex, 0, tr->p1());
        mk(HandleKind::Vertex, 1, tr->p2());
        mk(HandleKind::Vertex, 2, tr->p3());
    } else if (auto* pg = dynamic_cast<Polygon*>(shape_.get())) {
        const auto& pts = pg->points();
        for (int i=0;i<pts.size();++i) mk(HandleKind::Vertex, i, pts[i]);
    } else if (auto* pl = dynamic_cast<Polyline*>(shape_.get())) {
        const auto& pts = pl->points();
        for (int i=0;i<pts.size();++i) mk(HandleKind::Vertex, i, pts[i]);
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
    } else if (auto* el = dynamic_cast<Ellipse*>(shape_.get())) {
        const auto c = el->center();
        mk(HandleKind::Center, 0, c);
        mk(HandleKind::Radius, 0, c + QPointF(el->rx(), 0));
        mk(HandleKind::Radius, 1, c + QPointF(0, el->ry()));
    }

    // 旋转手柄：放在局部包围盒顶部中心上方 30px
    const auto br = boundingRect();
    QPointF topCenter = QPointF((br.left()+br.right())/2.0, br.top());
    const qreal rotSize = 16.0;
    rotationHandle_ = new ControlPointItem(this, ControlPointItem::Kind::Rotation, 0, QRectF(-rotSize/2, -rotSize/2, rotSize, rotSize));
    rotationHandle_->setPos(topCenter + QPointF(0, -30));
}

void ShapeItem::syncHandlesPositions(HandleKind activeKind, int activeIndex) {
    if (handles_.isEmpty() && !rotationHandle_) return;

    auto setIfNotActive = [&](ControlPointItem* h, const QPointF& p) {
        if (!h) return;
        if (static_cast<HandleKind>(h->kind()) == activeKind && h->index() == activeIndex) return;
        h->setPos(p);
    };

    if (auto* ls = dynamic_cast<LineSegment*>(shape_.get())) {
        for (auto* it : handles_) {
            auto* h = dynamic_cast<ControlPointItem*>(it);
            if (!h) continue;
            if (h->kind() != ControlPointItem::Kind::Vertex) continue;
            setIfNotActive(h, h->index() == 0 ? ls->p1() : ls->p2());
        }
    } else if (auto* tr = dynamic_cast<Triangle*>(shape_.get())) {
        for (auto* it : handles_) {
            auto* h = dynamic_cast<ControlPointItem*>(it);
            if (!h) continue;
            if (h->kind() != ControlPointItem::Kind::Vertex) continue;
            if (h->index() == 0) setIfNotActive(h, tr->p1());
            else if (h->index() == 1) setIfNotActive(h, tr->p2());
            else if (h->index() == 2) setIfNotActive(h, tr->p3());
        }
    } else if (auto* pg = dynamic_cast<Polygon*>(shape_.get())) {
        const auto& pts = pg->points();
        for (auto* it : handles_) {
            auto* h = dynamic_cast<ControlPointItem*>(it);
            if (!h) continue;
            if (h->kind() != ControlPointItem::Kind::Vertex) continue;
            if (h->index() >= 0 && h->index() < pts.size()) setIfNotActive(h, pts[h->index()]);
        }
    } else if (auto* pl = dynamic_cast<Polyline*>(shape_.get())) {
        const auto& pts = pl->points();
        for (auto* it : handles_) {
            auto* h = dynamic_cast<ControlPointItem*>(it);
            if (!h) continue;
            if (h->kind() != ControlPointItem::Kind::Vertex) continue;
            if (h->index() >= 0 && h->index() < pts.size()) setIfNotActive(h, pts[h->index()]);
        }
    } else if (auto* rc = dynamic_cast<Rectangle*>(shape_.get())) {
        const auto r = rc->rect().normalized();
        for (auto* it : handles_) {
            auto* h = dynamic_cast<ControlPointItem*>(it);
            if (!h) continue;
            if (h->kind() != ControlPointItem::Kind::Corner) continue;
            switch (h->index()) {
            case 0: setIfNotActive(h, r.topLeft()); break;
            case 1: setIfNotActive(h, r.topRight()); break;
            case 2: setIfNotActive(h, r.bottomRight()); break;
            case 3: setIfNotActive(h, r.bottomLeft()); break;
            default: break;
            }
        }
    } else if (auto* cc = dynamic_cast<Circle*>(shape_.get())) {
        const auto c = cc->center();
        for (auto* it : handles_) {
            auto* h = dynamic_cast<ControlPointItem*>(it);
            if (!h) continue;
            if (h->kind() == ControlPointItem::Kind::Center) setIfNotActive(h, c);
            else if (h->kind() == ControlPointItem::Kind::Radius) setIfNotActive(h, c + QPointF(cc->radius(), 0));
        }
    } else if (auto* el = dynamic_cast<Ellipse*>(shape_.get())) {
        const auto c = el->center();
        for (auto* it : handles_) {
            auto* h = dynamic_cast<ControlPointItem*>(it);
            if (!h) continue;
            if (h->kind() == ControlPointItem::Kind::Center) setIfNotActive(h, c);
            else if (h->kind() == ControlPointItem::Kind::Radius) {
                if (h->index() == 0) setIfNotActive(h, c + QPointF(el->rx(), 0));
                else if (h->index() == 1) setIfNotActive(h, c + QPointF(0, el->ry()));
            }
        }
    }

    if (rotationHandle_ && !(activeKind == HandleKind::Rotation)) {
        const auto br = boundingRect();
        QPointF topCenter = QPointF((br.left()+br.right())/2.0, br.top());
        rotationHandle_->setPos(topCenter + QPointF(0, -30));
    }
}

void ShapeItem::handleMoved(HandleKind kind, int index, const QPointF& localPos, const QPointF& /*scenePos*/, bool /*release*/) {
    if (auto* ls = dynamic_cast<LineSegment*>(shape_.get())) {
        if (kind == HandleKind::Vertex) {
            const QRectF oldBr = boundingRect();
            prepareGeometryChange();
            if (index == 0) ls->setP1(localPos); else ls->setP2(localPos);
            const QRectF newBr = boundingRect();
            update(oldBr.united(newBr));
            updateTransformOrigin();
            syncHandlesPositions(kind, index);
        }
    } else if (auto* tr = dynamic_cast<Triangle*>(shape_.get())) {
        if (kind == HandleKind::Vertex) {
            const QRectF oldBr = boundingRect();
            prepareGeometryChange();
            if (index == 0) tr->setP1(localPos);
            else if (index == 1) tr->setP2(localPos);
            else tr->setP3(localPos);
            const QRectF newBr = boundingRect();
            update(oldBr.united(newBr));
            updateTransformOrigin();
            syncHandlesPositions(kind, index);
        }
    } else if (auto* pg = dynamic_cast<Polygon*>(shape_.get())) {
        if (kind == HandleKind::Vertex) {
            const QRectF oldBr = boundingRect();
            prepareGeometryChange();
            auto pts = pg->points();
            if (index>=0 && index<pts.size()) pts[index] = localPos;
            pg->setPoints(pts);
            const QRectF newBr = boundingRect();
            update(oldBr.united(newBr));
            updateTransformOrigin();
            syncHandlesPositions(kind, index);
        }
    } else if (auto* pl = dynamic_cast<Polyline*>(shape_.get())) {
        if (kind == HandleKind::Vertex) {
            const QRectF oldBr = boundingRect();
            prepareGeometryChange();
            auto pts = pl->points();
            if (index>=0 && index<pts.size()) pts[index] = localPos;
            pl->setPoints(pts);
            const QRectF newBr = boundingRect();
            update(oldBr.united(newBr));
            updateTransformOrigin();
            syncHandlesPositions(kind, index);
        }
    } else if (auto* rc = dynamic_cast<Rectangle*>(shape_.get())) {
        if (kind == HandleKind::Corner) {
            const QRectF oldBr = boundingRect();
            prepareGeometryChange();
            const QRectF r = rc->rect().normalized();
            QPointF fixed;
            switch (index) {
            case 0: fixed = r.bottomRight(); break; // drag TL, fix BR
            case 1: fixed = r.bottomLeft(); break;  // drag TR, fix BL
            case 2: fixed = r.topLeft(); break;     // drag BR, fix TL
            case 3: fixed = r.topRight(); break;    // drag BL, fix TR
            default: fixed = r.bottomRight(); break;
            }

            // 旋转状态下，修改几何会改变中心点（transformOrigin），需要补偿位移以保持“固定对角点”在场景中不漂移
            const QPointF fixedSceneBefore = mapToScene(fixed);
            rc->setRect(QRectF(localPos, fixed).normalized());
            updateTransformOrigin();
            const QPointF fixedSceneAfter = mapToScene(fixed);
            const QPointF deltaScene = fixedSceneBefore - fixedSceneAfter;
            const QPointF deltaParent = parentItem()
                ? parentItem()->mapFromScene(fixedSceneBefore) - parentItem()->mapFromScene(fixedSceneAfter)
                : deltaScene;
            setPos(pos() + deltaParent);
            if (shape_) shape_->MoveTo(pos().x(), pos().y());

            const QRectF newBr = boundingRect();
            update(oldBr.united(newBr));
            syncHandlesPositions(kind, index);
        }
    } else if (auto* cc = dynamic_cast<Circle*>(shape_.get())) {
        if (kind == HandleKind::Center) {
            const QRectF oldBr = boundingRect();
            prepareGeometryChange();
            cc->setCenter(localPos);
            const QRectF newBr = boundingRect();
            update(oldBr.united(newBr));
            updateTransformOrigin();
            syncHandlesPositions(kind, index);
        } else if (kind == HandleKind::Radius) {
            const QRectF oldBr = boundingRect();
            prepareGeometryChange();
            const auto c = cc->center();
            const double r = std::hypot(localPos.x() - c.x(), localPos.y() - c.y());
            cc->setRadius(r);
            const QRectF newBr = boundingRect();
            update(oldBr.united(newBr));
            updateTransformOrigin();
            syncHandlesPositions(kind, index);
        }
    } else if (auto* el = dynamic_cast<Ellipse*>(shape_.get())) {
        if (kind == HandleKind::Center) {
            const QRectF oldBr = boundingRect();
            prepareGeometryChange();
            el->setCenter(localPos);
            const QRectF newBr = boundingRect();
            update(oldBr.united(newBr));
            updateTransformOrigin();
            syncHandlesPositions(kind, index);
        } else if (kind == HandleKind::Radius) {
            const QRectF oldBr = boundingRect();
            prepareGeometryChange();
            const auto c = el->center();
            if (index == 0) el->setRx(std::abs(localPos.x() - c.x()));
            else el->setRy(std::abs(localPos.y() - c.y()));
            const QRectF newBr = boundingRect();
            update(oldBr.united(newBr));
            updateTransformOrigin();
            syncHandlesPositions(kind, index);
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
        painter->save();
        painter->setClipRect(boundingRect());
        painter->drawText(boundingRect().topLeft() + QPointF(4, 12), QString("L=%1").arg(L, 0, 'f', 2));
        painter->restore();
        return;
    }
    if (auto* rc = dynamic_cast<Rectangle*>(shape_.get())) {
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(rc->rect());
        painter->setPen(QPen(Qt::darkGray));
        const double P = rc->Perimeter();
        const double A = rc->Area();
        painter->save();
        painter->setClipRect(boundingRect());
        painter->drawText(rc->rect().topLeft() + QPointF(4, 12), QString("P=%1 A=%2").arg(P, 0, 'f', 2).arg(A, 0, 'f', 2));
        painter->restore();
        return;
    }
    if (auto* cc = dynamic_cast<Circle*>(shape_.get())) {
        painter->setBrush(Qt::NoBrush);
        const auto r = cc->radius();
        painter->drawEllipse(cc->center(), r, r);
        painter->setPen(QPen(Qt::darkGray));
        const double P = cc->Perimeter();
        const double A = cc->Area();
        painter->save();
        painter->setClipRect(boundingRect());
        painter->drawText(boundingRect().topLeft() + QPointF(4, 12), QString("P=%1 A=%2").arg(P, 0, 'f', 2).arg(A, 0, 'f', 2));
        painter->restore();
        return;
    }
    if (auto* tr = dynamic_cast<Triangle*>(shape_.get())) {
        painter->setBrush(Qt::NoBrush);
        QPolygonF poly; poly << tr->p1() << tr->p2() << tr->p3();
        painter->drawPolygon(poly);
        painter->setPen(QPen(Qt::darkGray));
        painter->save();
        painter->setClipRect(boundingRect());
        painter->drawText(boundingRect().topLeft() + QPointF(4, 12), QString("P=%1 A=%2").arg(tr->Perimeter(), 0, 'f', 2).arg(tr->Area(), 0, 'f', 2));
        painter->restore();
        return;
    }
    if (auto* pg = dynamic_cast<Polygon*>(shape_.get())) {
        painter->setBrush(Qt::NoBrush);
        painter->drawPolygon(QPolygonF(pg->points()));
        painter->setPen(QPen(Qt::darkGray));
        painter->save();
        painter->setClipRect(boundingRect());
        painter->drawText(boundingRect().topLeft() + QPointF(4, 12), QString("P=%1 A=%2").arg(pg->Perimeter(), 0, 'f', 2).arg(pg->Area(), 0, 'f', 2));
        painter->restore();
        return;
    }
    if (auto* pl = dynamic_cast<Polyline*>(shape_.get())) {
        painter->setBrush(Qt::NoBrush);
        painter->drawPolyline(QPolygonF(pl->points()));
        painter->setPen(QPen(Qt::darkGray));
        painter->save();
        painter->setClipRect(boundingRect());
        painter->drawText(boundingRect().topLeft() + QPointF(4, 12), QString("L=%1").arg(pl->Length(), 0, 'f', 2));
        painter->restore();
        return;
    }
    if (auto* el = dynamic_cast<Ellipse*>(shape_.get())) {
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(el->center(), el->rx(), el->ry());
        painter->setPen(QPen(Qt::darkGray));
        painter->save();
        painter->setClipRect(boundingRect());
        painter->drawText(boundingRect().topLeft() + QPointF(4, 12), QString("P=%1 A=%2").arg(el->Perimeter(), 0, 'f', 2).arg(el->Area(), 0, 'f', 2));
        painter->restore();
        return;
    }
}
