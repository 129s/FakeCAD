#pragma once

#include <QGraphicsScene>
#include <QPointF>
#include <QVector>

class QUndoStack;

class QGraphicsLineItem;
class QGraphicsRectItem;
class QGraphicsEllipseItem;
class QGraphicsPathItem;

class DrawingScene : public QGraphicsScene {
    Q_OBJECT
public:
    enum class Mode { None, Line, Rect, Circle, Ellipse, Polygon, Triangle };

    explicit DrawingScene(QObject* parent = nullptr);

    void setMode(Mode m);
    Mode mode() const { return mode_; }

    // Grid & snapping
    void setShowGrid(bool v) { showGrid_ = v; update(); }
    bool showGrid() const { return showGrid_; }
    void setSnapToGrid(bool v) { snapToGrid_ = v; }
    bool snapToGrid() const { return snapToGrid_; }
    void setGridSize(qreal s) { gridSize_ = s; update(); }
    qreal gridSize() const { return gridSize_; }
    QPointF snapPoint(const QPointF& p) const;
    void setUndoStack(QUndoStack* s) { undo_ = s; }
    QUndoStack* undoStack() const { return undo_; }

 protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    void drawBackground(QPainter* painter, const QRectF& rect) override;        

private:
    Mode mode_ { Mode::None };
    bool drawing_ { false };
    QPointF startPos_ {};

    QGraphicsLineItem*    previewLine_   { nullptr };
    QGraphicsRectItem*    previewRect_   { nullptr };
    QGraphicsEllipseItem* previewCircle_ { nullptr };
    QGraphicsEllipseItem* previewEllipse_ { nullptr };
    QGraphicsPathItem* previewPolygon_ { nullptr };
    QVector<QPointF> polygonPoints_ {};
    QGraphicsPathItem* previewTriangle_ { nullptr };
    QVector<QPointF> trianglePoints_ {};

    void clearPreview();
    void updatePolygonPreview(const QPointF& cur);
    void finishPolygon();
    void updateTrianglePreview(const QPointF& cur);

    // grid settings
    bool showGrid_ { true };
    bool snapToGrid_ { false };
    qreal gridSize_ { 20.0 };

    class QUndoStack* undo_ { nullptr };
};
