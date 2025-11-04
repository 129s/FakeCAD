#pragma once

#include <QString>
#include <QColor>
#include <QPen>
#include <QTransform>
#include <QJsonObject>
#include <QRectF>
#include <QVector>
#include <QPointF>

class Shape {
public:
    virtual ~Shape() = default;

    // 类型名（运行时标识）
    virtual QString typeName() const = 0;

    // 通用属性
    const QString& name() const { return name_; }
    void setName(const QString& n) { name_ = n; }

    const QColor& color() const { return color_; }
    void setColor(const QColor& c) { color_ = c; }

    const QPen& pen() const { return pen_; }
    void setPen(const QPen& p) { pen_ = p; }

    const QTransform& transform() const { return transform_; }
    void setTransform(const QTransform& t) { transform_ = t; }

    // 变换操作
    virtual void Move(double dx, double dy) { transform_.translate(dx, dy); }
    virtual void MoveTo(double x, double y) {
        // 将平移部分设置为 (x, y)，其余变换保持
        QTransform t = transform_;
        t.setMatrix(t.m11(), t.m12(), t.m13(),
                    t.m21(), t.m22(), t.m23(),
                    x,       y,       t.m33());
        transform_ = t;
    }
    virtual void Rotate(double angleDeg) {
        transform_.rotate(angleDeg);
    }

    // 度量接口
    virtual double Length() const { return 0.0; }

    // 包围盒（局部+变换后）
    virtual QRectF BoundingBox() const = 0;

    // 序列化（通用字段）
    virtual QJsonObject ToJson() const;
    virtual void FromJsonCommon(const QJsonObject& obj);

protected:
    QString name_;
    QColor color_{Qt::black};
    QPen pen_{QPen(Qt::black)};
    QTransform transform_{};
};

class LineShape : public Shape {
public:
    ~LineShape() override = default;

    virtual int VertexCount() const = 0;
    virtual QVector<QPointF> Vertices() const = 0;

    double Length() const override;
};

class AreaShape : public Shape {
public:
    ~AreaShape() override = default;
    virtual double Area() const = 0;
    virtual double Perimeter() const = 0;
    double Length() const override { return Perimeter(); }
};

