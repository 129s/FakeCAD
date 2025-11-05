#include "CanvasView.h"

#include <QWheelEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QDebug>
#include <QGuiApplication>
#include <algorithm>

static bool fc_debug_input_enabled() {
    static bool on = !qEnvironmentVariableIsEmpty("FAKECAD_DEBUG_INPUT");
    return on;
}

static const char* cursorShapeName(Qt::CursorShape s) {
    switch (s) {
    case Qt::ArrowCursor: return "Arrow";
    case Qt::OpenHandCursor: return "OpenHand";
    case Qt::ClosedHandCursor: return "ClosedHand";
    case Qt::SizeAllCursor: return "SizeAll";
    case Qt::CrossCursor: return "Cross";
    default: return "Other";
    }
}

static const char* dragModeName(QGraphicsView::DragMode m) {
    switch (m) {
    case QGraphicsView::NoDrag: return "NoDrag";
    case QGraphicsView::RubberBandDrag: return "RubberBandDrag";
    case QGraphicsView::ScrollHandDrag: return "ScrollHandDrag";
    default: return "Unknown";
    }
}

bool CanvasView::viewportEvent(QEvent* event) {
    if (fc_debug_input_enabled()) {
        if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::MouseMove) {
            auto* me = static_cast<QMouseEvent*>(event);
            const Qt::CursorShape vshape = viewport()->cursor().shape();
            const QCursor* oc = QGuiApplication::overrideCursor();
            qInfo() << "[INPUT]" << (event->type()==QEvent::MouseButtonPress?"Press":event->type()==QEvent::MouseButtonRelease?"Release":"Move")
                    << "btn:" << me->button() << "btns:" << me->buttons()
                    << "drag:" << dragModeName(dragMode())
                    << "spacePan:" << spacePanning_
                    << "vpCursor:" << cursorShapeName(vshape)
                    << "override:" << (oc?cursorShapeName(oc->shape()):"none");
        }
    }
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
