#pragma once

#include <QGraphicsView>

class CanvasView : public QGraphicsView {
    Q_OBJECT
public:
    explicit CanvasView(QWidget* parent = nullptr);
    explicit CanvasView(QGraphicsScene* scene, QWidget* parent = nullptr);

signals:
    void mouseScenePosChanged(const QPointF& pos);

protected:
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

private:
    qreal zoomFactor_ { 1.15 };
    qreal minScale_ { 0.1 };
    qreal maxScale_ { 10.0 };
    bool spacePanning_ { false };
    bool midPanning_ { false };
    DragMode savedDragMode_ { QGraphicsView::NoDrag };
    QPoint lastPanPos_;

    void beginPan();
    void endPan();

public:
    void zoomBy(qreal factor);
    void resetZoom();
};
