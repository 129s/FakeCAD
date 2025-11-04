#pragma once

#include <QGraphicsScene>
#include <QPointF>

class QGraphicsLineItem;
class QGraphicsRectItem;
class QGraphicsEllipseItem;

class DrawingScene : public QGraphicsScene {
    Q_OBJECT
public:
    enum class Mode { None, Line, Rect, Circle };

    explicit DrawingScene(QObject* parent = nullptr);

    void setMode(Mode m);
    Mode mode() const { return mode_; }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    Mode mode_ { Mode::None };
    bool drawing_ { false };
    QPointF startPos_ {};

    QGraphicsLineItem*    previewLine_   { nullptr };
    QGraphicsRectItem*    previewRect_   { nullptr };
    QGraphicsEllipseItem* previewCircle_ { nullptr };

    void clearPreview();
};

