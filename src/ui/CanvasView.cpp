#include "CanvasView.h"

#include <QWheelEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <algorithm>

CanvasView::CanvasView(QWidget* parent)
    : QGraphicsView(parent) {
    setRenderHint(QPainter::Antialiasing, true);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
}

CanvasView::CanvasView(QGraphicsScene* scene, QWidget* parent)
    : QGraphicsView(scene, parent) {
    setRenderHint(QPainter::Antialiasing, true);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
}

void CanvasView::wheelEvent(QWheelEvent* event) {
    if (event->angleDelta().y() == 0) { QGraphicsView::wheelEvent(event); return; }
    const int delta = event->angleDelta().y();
    const qreal factor = delta > 0 ? zoomFactor_ : (1.0 / zoomFactor_);
    zoomBy(factor);
}

void CanvasView::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::MiddleButton) {
        // 显式禁用中键拖拽等残留手势
        event->accept();
        return;
    }
    if (spacePanning_ && event->button() == Qt::LeftButton) {
        setCursor(Qt::ClosedHandCursor);
    }
    QGraphicsView::mousePressEvent(event);
}

void CanvasView::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::MiddleButton) {
        event->accept();
        return;
    }
    if (spacePanning_ && event->button() == Qt::LeftButton) {
        setCursor(Qt::OpenHandCursor);
    }
    QGraphicsView::mouseReleaseEvent(event);
}

void CanvasView::mouseMoveEvent(QMouseEvent* event) {
    if (event->buttons() & Qt::MiddleButton) {
        // 屏蔽按住中键移动（不触发任何手势）
        event->accept();
        if (scene()) emit mouseScenePosChanged(mapToScene(event->pos()));
        return;
    }
    if (scene()) emit mouseScenePosChanged(mapToScene(event->pos()));
    QGraphicsView::mouseMoveEvent(event);
}

void CanvasView::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Space && !spacePanning_) {
        spacePanning_ = true;
        beginPan();
        return;
    }
    QGraphicsView::keyPressEvent(event);
}

void CanvasView::keyReleaseEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Space && spacePanning_) {
        spacePanning_ = false;
        endPan();
        return;
    }
    QGraphicsView::keyReleaseEvent(event);
}

void CanvasView::beginPan() {
    savedDragMode_ = dragMode();
    setDragMode(QGraphicsView::ScrollHandDrag);
    setCursor(Qt::OpenHandCursor);
}

void CanvasView::endPan() {
    setDragMode(savedDragMode_);
    unsetCursor();
}

void CanvasView::zoomBy(qreal factor) {
    // clamp to min/max scale (assume uniform scale)
    const qreal cur = transform().m11();
    qreal next = cur * factor;
    next = std::clamp(next, minScale_, maxScale_);
    const qreal apply = next / cur;
    scale(apply, apply);
}

void CanvasView::resetZoom() {
    resetTransform();
}
