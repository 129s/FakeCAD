#pragma once

#include <memory>
#include <QGraphicsItem>
#include "../core/Shape.h"
#include "../core/shapes/LineSegment.h"
#include "../core/shapes/Rectangle.h"
#include "../core/shapes/Circle.h"

class ShapeItem : public QGraphicsItem {
public:
    explicit ShapeItem(std::unique_ptr<Shape> shape, QGraphicsItem* parent = nullptr);
    ~ShapeItem() override = default;

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    Shape* shape() const { return shape_.get(); }

private:
    std::unique_ptr<Shape> shape_;
};

