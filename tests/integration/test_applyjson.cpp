// 集成：针对所有形状测试 ApplyJsonToShape 与 FromJsonObject
#define MINI_TEST_MAIN 1
#include "minitest.h"

#include "core/Serialization.h"
#include <QJsonArray>
#include "core/shapes/LineSegment.h"
#include "core/shapes/Rectangle.h"
#include "core/shapes/Circle.h"
#include "core/shapes/Triangle.h"
#include "core/shapes/Polygon.h"
#include "core/shapes/Polyline.h"
#include "core/shapes/Ellipse.h"

TEST_CASE("ApplyJson LineSegment") {
    LineSegment ls({0,0},{1,1});
    auto j = ls.ToJson(); j["type"] = QStringLiteral("LineSegment");
    auto g = j["geom"].toObject(); g["x2"] = 3.0; g["y2"] = 4.0; j["geom"] = g;
    REQUIRE(Ser::ApplyJsonToShape(&ls, j));
    REQUIRE_NEAR(ls.Length(), 5.0, 1e-9);
}

TEST_CASE("ApplyJson Rectangle") {
    Rectangle rc(QRectF(0,0,1,1)); auto j = rc.ToJson(); j["type"] = QStringLiteral("Rectangle");
    auto g = j["geom"].toObject(); g["w"] = 10.0; g["h"] = 20.0; j["geom"] = g;
    REQUIRE(Ser::ApplyJsonToShape(&rc, j));
    REQUIRE_NEAR(rc.Area(), 200.0, 1e-9);
}

TEST_CASE("ApplyJson Circle") {
    Circle c(QPointF(0,0), 1.0); auto j = c.ToJson(); j["type"] = QStringLiteral("Circle");
    auto g = j["geom"].toObject(); g["r"] = 7.0; j["geom"] = g;
    REQUIRE(Ser::ApplyJsonToShape(&c, j));
    REQUIRE_NEAR(c.radius(), 7.0, 1e-9);
}

TEST_CASE("ApplyJson Triangle") {
    Triangle t(QPointF(0,0), QPointF(1,0), QPointF(0,1)); auto j = t.ToJson(); j["type"] = QStringLiteral("Triangle");
    auto g = j["geom"].toObject(); g["x3"] = 0.0; g["y3"] = 3.0; j["geom"] = g;
    REQUIRE(Ser::ApplyJsonToShape(&t, j));
    REQUIRE_NEAR(t.Area(), 1.5, 1e-9);
}

TEST_CASE("ApplyJson Polygon") {
    Polygon p({QPointF(0,0), QPointF(1,0), QPointF(1,1)}); auto j = p.ToJson(); j["type"] = QStringLiteral("Polygon");
    auto g = j["geom"].toObject(); QJsonArray arr = g["points"].toArray(); arr.append(QJsonObject{{"x",0.0},{"y",1.0}}); g["points"] = arr; j["geom"] = g;
    REQUIRE(Ser::ApplyJsonToShape(&p, j));
    REQUIRE_NEAR(p.Perimeter(), 4.0, 1e-9);
}

TEST_CASE("ApplyJson Polyline") {
    Polyline pl({QPointF(0,0), QPointF(1,0)}); auto j = pl.ToJson(); j["type"] = QStringLiteral("Polyline");
    auto g = j["geom"].toObject(); QJsonArray arr = g["points"].toArray(); arr.append(QJsonObject{{"x",3.0},{"y",4.0}}); g["points"] = arr; j["geom"] = g;
    REQUIRE(Ser::ApplyJsonToShape(&pl, j));
    const double expected = 1.0 + std::hypot(3.0-1.0, 4.0-0.0);
    REQUIRE_NEAR(pl.Length(), expected, 1e-9);
}

TEST_CASE("ApplyJson Ellipse") {
    Ellipse e(QPointF(0,0), 2.0, 3.0); auto j = e.ToJson(); j["type"] = QStringLiteral("Ellipse");
    auto g = j["geom"].toObject(); g["rx"] = 5.0; g["ry"] = 6.0; j["geom"] = g;
    REQUIRE(Ser::ApplyJsonToShape(&e, j));
    REQUIRE_NEAR(e.rx(), 5.0, 1e-9);
    REQUIRE_NEAR(e.ry(), 6.0, 1e-9);
}
