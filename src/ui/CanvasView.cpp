#include "CanvasView.h"

#include <QWheelEvent>
#include <QMouseEvent>
#include <QKeyEvent>

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
    scale(factor, factor);
}

void CanvasView::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::MiddleButton) {
        beginPan();
        event->accept();
        return;
    }
    QGraphicsView::mousePressEvent(event);
}

void CanvasView::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::MiddleButton) {
        endPan();
        event->accept();
        return;
    }
    QGraphicsView::mouseReleaseEvent(event);
}

void CanvasView::mouseMoveEvent(QMouseEvent* event) {
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
}

void CanvasView::endPan() {
    setDragMode(savedDragMode_);
}

