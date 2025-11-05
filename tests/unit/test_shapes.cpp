// 单元测试：核心几何/基类行为
#define MINI_TEST_MAIN 1
#include "minitest.h"

#include <QtCore/QString>
#include <QtGui/QColor>
#include <QtGui/QPen>

#include "core/Shape.h"
#include "core/shapes/LineSegment.h"
#include "core/shapes/Rectangle.h"
#include "core/shapes/Circle.h"
#include "core/shapes/Triangle.h"
#include "core/shapes/Polygon.h"
#include "core/shapes/Polyline.h"
#include "core/shapes/Ellipse.h"

TEST_CASE("LineSegment length") {
    LineSegment ls({0,0},{3,4});
    REQUIRE_NEAR(ls.Length(), 5.0, 1e-9);
}

TEST_CASE("Rectangle area and perimeter") {
    Rectangle r(QRectF(0,0,10,20));
    REQUIRE_NEAR(r.Area(), 200.0, 1e-9);
    REQUIRE_NEAR(r.Perimeter(), 60.0, 1e-9);
}

TEST_CASE("Circle area/perimeter and clamp radius") {
    Circle c(QPointF(0,0), 10.0);
    REQUIRE_NEAR(c.Area(), 3.14159265358979323846*100.0, 1e-9);
    REQUIRE_NEAR(c.Perimeter(), 2.0*3.14159265358979323846*10.0, 1e-9);
    c.setRadius(-5.0);
    REQUIRE(c.radius() >= 0.0);
}

TEST_CASE("Triangle area/perimeter") {
    Triangle t({0,0},{4,0},{0,3});
    REQUIRE_NEAR(t.Area(), 6.0, 1e-9);
    REQUIRE_NEAR(t.Perimeter(), 12.0, 1e-9);
}

TEST_CASE("Polygon area/perimeter (square)") {
    Polygon p({QPointF(0,0), QPointF(2,0), QPointF(2,2), QPointF(0,2)});
    REQUIRE_NEAR(p.Area(), 4.0, 1e-9);
    REQUIRE_NEAR(p.Perimeter(), 8.0, 1e-9);
}

TEST_CASE("Polyline length") {
    Polyline pl({QPointF(0,0), QPointF(3,4), QPointF(6,4)});
    REQUIRE_NEAR(pl.Length(), 5.0 + 3.0, 1e-9);
}

TEST_CASE("Ellipse area and degenerate perimeter") {
    Ellipse e(QPointF(0,0), 10.0, 0.0);
    REQUIRE_NEAR(e.Area(), 0.0, 1e-9);
    REQUIRE_NEAR(e.Perimeter(), 0.0, 1e-9);
    Ellipse e2(QPointF(0,0), 3.0, 4.0);
    REQUIRE_NEAR(e2.Area(), 3.14159265358979323846*12.0, 1e-9);
}

TEST_CASE("Shape transform and rotation") {
    Rectangle r(QRectF(0,0,1,1));
    r.Move(5, -2);
    REQUIRE_NEAR(r.transform().m31(), 5.0, 1e-9);
    REQUIRE_NEAR(r.transform().m32(), -2.0, 1e-9);
    r.MoveTo(10, 20);
    REQUIRE_NEAR(r.transform().m31(), 10.0, 1e-9);
    REQUIRE_NEAR(r.transform().m32(), 20.0, 1e-9);
    r.setRotationDegrees(15.0);
    REQUIRE_NEAR(r.rotationDegrees(), 15.0, 1e-9);
}
