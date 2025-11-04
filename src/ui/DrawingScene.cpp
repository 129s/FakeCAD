#include "DrawingScene.h"

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QtMath>
#include <cmath>

#include "ShapeItem.h"
#include "../core/shapes/LineSegment.h"
#include "../core/shapes/Rectangle.h"
#include "../core/shapes/Circle.h"
#include "../undo/Commands.h"
#include "../core/Serialization.h"

DrawingScene::DrawingScene(QObject* parent)
    : QGraphicsScene(parent) {
}

void DrawingScene::setMode(Mode m) {
    if (mode_ == m) return;
    mode_ = m;
    clearPreview();
}

void DrawingScene::clearPreview() {
    if (previewLine_) { removeItem(previewLine_); delete previewLine_; previewLine_ = nullptr; }
    if (previewRect_) { removeItem(previewRect_); delete previewRect_; previewRect_ = nullptr; }
    if (previewCircle_) { removeItem(previewCircle_); delete previewCircle_; previewCircle_ = nullptr; }
    drawing_ = false;
}

void DrawingScene::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton && mode_ != Mode::None) {
        drawing_ = true;
        startPos_ = snapPoint(event->scenePos());
        switch (mode_) {
        case Mode::Line:
            previewLine_ = addLine(QLineF(startPos_, startPos_), QPen(Qt::darkGray, 1, Qt::DashLine));
            break;
        case Mode::Rect:
            previewRect_ = addRect(QRectF(startPos_, startPos_), QPen(Qt::darkGray, 1, Qt::DashLine));
            break;
        case Mode::Circle:
            previewCircle_ = addEllipse(QRectF(startPos_, startPos_), QPen(Qt::darkGray, 1, Qt::DashLine));
            break;
        case Mode::Ellipse:
            previewEllipse_ = addEllipse(QRectF(startPos_, startPos_), QPen(Qt::darkGray, 1, Qt::DashLine));
            break;
        default:
            break;
        }
        event->accept();
        return;
    }
    QGraphicsScene::mousePressEvent(event);
}

void DrawingScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    if (drawing_) {
        const QPointF cur = snapPoint(event->scenePos());
        switch (mode_) {
        case Mode::Line:
            if (previewLine_) previewLine_->setLine(QLineF(startPos_, cur));
            break;
        case Mode::Rect: {
            if (previewRect_) previewRect_->setRect(QRectF(startPos_, cur).normalized());
            break;
        }
        case Mode::Circle: {
            if (previewCircle_) {
                const qreal r = std::hypot(cur.x() - startPos_.x(), cur.y() - startPos_.y());
                QRectF box(startPos_.x() - r, startPos_.y() - r, 2*r, 2*r);
                previewCircle_->setRect(box);
            }
            break;
        }
        case Mode::Ellipse: {
            if (previewEllipse_) {
                QRectF box(startPos_, cur);
                box = box.normalized();
                // center 固定为 startPos_
                previewEllipse_->setRect(QRectF(startPos_.x() - std::abs(cur.x()-startPos_.x()),
                                                startPos_.y() - std::abs(cur.y()-startPos_.y()),
                                                2*std::abs(cur.x()-startPos_.x()),
                                                2*std::abs(cur.y()-startPos_.y())));
            }
            break;
        }
        default:
            break;
        }
        event->accept();
        return;
    }
    QGraphicsScene::mouseMoveEvent(event);
}

void DrawingScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    if (drawing_ && event->button() == Qt::LeftButton) {
        const QPointF endPos = snapPoint(event->scenePos());
        auto dist = [](const QPointF& a, const QPointF& b){ return std::hypot(a.x()-b.x(), a.y()-b.y()); };
        const qreal eps = 1.0; // 最小尺寸阈值（像素）
        switch (mode_) {
        case Mode::Line: {
            if (previewLine_) { removeItem(previewLine_); delete previewLine_; previewLine_ = nullptr; }
            if (dist(startPos_, endPos) >= eps) {
                // 通过命令创建
                LineSegment tmp(startPos_, endPos);
                auto json = tmp.ToJson(); json["type"] = QStringLiteral("LineSegment");
                if (undo_) undo_->push(new UndoCmd::AddShapeCommand(this, json));
                else addItem(new ShapeItem(std::make_unique<LineSegment>(startPos_, endPos)));
            }
            break;
        }
        case Mode::Rect: {
            if (previewRect_) { removeItem(previewRect_); delete previewRect_; previewRect_ = nullptr; }
            QRectF r = QRectF(startPos_, endPos).normalized();
            if (r.width() >= eps && r.height() >= eps) {
                Rectangle tmp(r);
                auto json = tmp.ToJson(); json["type"] = QStringLiteral("Rectangle");
                if (undo_) undo_->push(new UndoCmd::AddShapeCommand(this, json));
                else addItem(new ShapeItem(std::make_unique<Rectangle>(r)));
            }
            break;
        }
        case Mode::Circle: {
            if (previewCircle_) { removeItem(previewCircle_); delete previewCircle_; previewCircle_ = nullptr; }
            const qreal r = std::hypot(endPos.x() - startPos_.x(), endPos.y() - startPos_.y());
            if (r >= eps) {
                Circle tmp(startPos_, r);
                auto json = tmp.ToJson(); json["type"] = QStringLiteral("Circle");
                if (undo_) undo_->push(new UndoCmd::AddShapeCommand(this, json));
                else addItem(new ShapeItem(std::make_unique<Circle>(startPos_, r)));
            }
            break;
        }
        case Mode::Ellipse: {
            if (previewEllipse_) { removeItem(previewEllipse_); delete previewEllipse_; previewEllipse_ = nullptr; }
            const qreal rx = std::abs(endPos.x() - startPos_.x());
            const qreal ry = std::abs(endPos.y() - startPos_.y());
            if (rx >= eps && ry >= eps) {
                Ellipse tmp(startPos_, rx, ry);
                auto json = tmp.ToJson(); json["type"] = QStringLiteral("Ellipse");
                if (undo_) undo_->push(new UndoCmd::AddShapeCommand(this, json));
                else addItem(new ShapeItem(std::make_unique<Ellipse>(startPos_, rx, ry)));
            }
            break;
        }
        default:
            break;
        }
        drawing_ = false;
        event->accept();
        return;
    }
    QGraphicsScene::mouseReleaseEvent(event);
}

QPointF DrawingScene::snapPoint(const QPointF& p) const {
    if (!snapToGrid_) return p;
    const qreal s = gridSize_ <= 0 ? 1.0 : gridSize_;
    const qreal x = std::round(p.x() / s) * s;
    const qreal y = std::round(p.y() / s) * s;
    return QPointF(x, y);
}

void DrawingScene::drawBackground(QPainter* painter, const QRectF& rect) {
    QGraphicsScene::drawBackground(painter, rect);
    if (!showGrid_) return;
    const qreal s = gridSize_ <= 0 ? 20.0 : gridSize_;
    painter->save();
    QPen gridPen(QColor(230,230,230));
    gridPen.setCosmetic(true); // width not scaled by zoom
    painter->setPen(gridPen);
    const qreal left = std::floor(rect.left() / s) * s;
    const qreal top  = std::floor(rect.top() / s) * s;
    for (qreal x = left; x < rect.right(); x += s) painter->drawLine(QPointF(x, rect.top()), QPointF(x, rect.bottom()));
    for (qreal y = top;  y < rect.bottom(); y += s) painter->drawLine(QPointF(rect.left(), y), QPointF(rect.right(), y));
    painter->restore();
}
