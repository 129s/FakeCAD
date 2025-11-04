#include "DrawingScene.h"

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QtMath>

#include "ShapeItem.h"
#include "../core/shapes/LineSegment.h"
#include "../core/shapes/Rectangle.h"
#include "../core/shapes/Circle.h"

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
        startPos_ = event->scenePos();
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
        const QPointF cur = event->scenePos();
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
        const QPointF endPos = event->scenePos();
        switch (mode_) {
        case Mode::Line: {
            if (previewLine_) { removeItem(previewLine_); delete previewLine_; previewLine_ = nullptr; }
            addItem(new ShapeItem(std::make_unique<LineSegment>(startPos_, endPos)));
            break;
        }
        case Mode::Rect: {
            if (previewRect_) { removeItem(previewRect_); delete previewRect_; previewRect_ = nullptr; }
            addItem(new ShapeItem(std::make_unique<Rectangle>(QRectF(startPos_, endPos).normalized())));
            break;
        }
        case Mode::Circle: {
            if (previewCircle_) { removeItem(previewCircle_); delete previewCircle_; previewCircle_ = nullptr; }
            const qreal r = std::hypot(endPos.x() - startPos_.x(), endPos.y() - startPos_.y());
            addItem(new ShapeItem(std::make_unique<Circle>(startPos_, r)));
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

