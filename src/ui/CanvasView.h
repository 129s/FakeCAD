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
    bool spacePanning_ { false };
    DragMode savedDragMode_ { QGraphicsView::NoDrag };

    void beginPan();
    void endPan();
};

