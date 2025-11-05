// 集成测试：序列化/反序列化/文件 I/O
#define MINI_TEST_MAIN 1
#include "minitest.h"

#include <QtCore/QTemporaryDir>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>

#include "core/Serialization.h"
#include "core/shapes/LineSegment.h"
#include "core/shapes/Rectangle.h"
#include "core/shapes/Circle.h"

static int countType(const std::vector<std::unique_ptr<Shape>>& v, const QString& t) {
    int c = 0; for (auto& s : v) if (s && s->typeName() == t) ++c; return c;
}

TEST_CASE("Serialize/Deserialize roundtrip") {
    LineSegment ls({0,0},{1,1}); ls.setName("A");
    Rectangle rc(QRectF(0,0,10,20)); rc.setName("B");
    Circle cc(QPointF(5,5), 3.0); cc.setName("C");
    std::vector<Shape*> in { &ls, &rc, &cc };

    auto doc = Ser::Serialize(in);
    auto out = Ser::Deserialize(doc);

    REQUIRE(out.size() == 3);
    REQUIRE(countType(out, "LineSegment") == 1);
    REQUIRE(countType(out, "Rectangle") == 1);
    REQUIRE(countType(out, "Circle") == 1);
}

TEST_CASE("SaveToFile/LoadFromFile") {
    QTemporaryDir tmp;
    REQUIRE(tmp.isValid());
    const QString path = tmp.filePath("a.json");

    LineSegment ls({0,0},{2,0});
    Rectangle rc(QRectF(1,2,3,4));
    std::vector<Shape*> in { &ls, &rc };

    QString err;
    REQUIRE(Ser::SaveToFile(path, in, &err));
    REQUIRE(err.isEmpty());
    REQUIRE(QFileInfo::exists(path));

    auto out = Ser::LoadFromFile(path, &err);
    REQUIRE(err.isEmpty());
    REQUIRE(out.size() == 2);
    REQUIRE(countType(out, "LineSegment") == 1);
    REQUIRE(countType(out, "Rectangle") == 1);
}
