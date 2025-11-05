#include "CanvasView.h"

#include <QWheelEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <algorithm>

bool CanvasView::viewportEvent(QEvent* event) {
    if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::MouseMove) {
        auto* me = static_cast<QMouseEvent*>(event);
        // 在最早的 viewportEvent 层屏蔽中键相关事件，避免底层改变手型光标/触发拖拽
        if ((event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease) && me->button() == Qt::MiddleButton) {
            unsetCursor();
            event->accept();
            return true;
        }
        if (event->type() == QEvent::MouseMove && (me->buttons() & Qt::MiddleButton)) {
            if (scene()) emit mouseScenePosChanged(mapToScene(me->pos()));
            unsetCursor();
            event->accept();
            return true;
        }
    }
    return QGraphicsView::viewportEvent(event);
}

CanvasView::CanvasView(QWidget* parent)
    : QGraphicsView(parent) {
    setRenderHint(QPainter::Antialiasing, true);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
}

CanvasView::CanvasView(QGraphicsScene* scene, QWidget* parent)
    : QGraphicsView(scene, parent) {
    setRenderHint(QPainter::Antialiasing, true);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
}

void CanvasView::wheelEvent(QWheelEvent* event) {
    if (event->angleDelta().y() == 0) { QGraphicsView::wheelEvent(event); return; }
    const int delta = event->angleDelta().y();
    const qreal factor = delta > 0 ? zoomFactor_ : (1.0 / zoomFactor_);
    zoomBy(factor);
}

void CanvasView::mousePressEvent(QMouseEvent* event) {
    if (spacePanning_ && event->button() == Qt::LeftButton) {
        setCursor(Qt::ClosedHandCursor);
    }
    QGraphicsView::mousePressEvent(event);
}

void CanvasView::mouseReleaseEvent(QMouseEvent* event) {
    if (spacePanning_ && event->button() == Qt::LeftButton) {
        setCursor(Qt::OpenHandCursor);
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

void CanvasView::focusOutEvent(QFocusEvent* event) {
    if (spacePanning_) { spacePanning_ = false; endPan(); }
    QGraphicsView::focusOutEvent(event);
}

void CanvasView::leaveEvent(QEvent* event) {
    if (spacePanning_) { spacePanning_ = false; endPan(); }
    QGraphicsView::leaveEvent(event);
}
