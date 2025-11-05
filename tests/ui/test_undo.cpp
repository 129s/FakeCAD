// UI：测试撤销/重做命令（添加/删除/变换/编辑几何）
#include <QtTest/QtTest>
#include <QUndoStack>
#include <QJsonObject>

#include "ui/DrawingScene.h"
#include "ui/ShapeItem.h"
#include "core/Serialization.h"
#include "core/shapes/Rectangle.h"
#include "core/shapes/Circle.h"
#include "undo/Commands.h"

static int shapeItemCount(QGraphicsScene* s) {
    int c=0; for (auto* it : s->items()) if (dynamic_cast<ShapeItem*>(it)) ++c; return c;
}

class UndoCommandsTest : public QObject {
    Q_OBJECT
private slots:
    void add_and_undo();
    void transform_and_undo();
    void edit_json_and_undo();
    void delete_and_undo();
};

void UndoCommandsTest::add_and_undo() {
    DrawingScene scene; QUndoStack stack; scene.setUndoStack(&stack);
    Rectangle tmp(QRectF(0,0,20,10)); QJsonObject j = tmp.ToJson();
    j["type"] = QStringLiteral("Rectangle");
    const int before = shapeItemCount(&scene);
    stack.push(new UndoCmd::AddShapeCommand(&scene, j));
    QCOMPARE(shapeItemCount(&scene), before + 1);
    stack.undo();
    QCOMPARE(shapeItemCount(&scene), before);
    stack.redo();
    QCOMPARE(shapeItemCount(&scene), before + 1);
}

void UndoCommandsTest::transform_and_undo() {
    DrawingScene scene; QUndoStack stack; scene.setUndoStack(&stack);
    auto* item = new ShapeItem(std::make_unique<Rectangle>(QRectF(0,0,10,10)));
    scene.addItem(item);
    const QPointF oldPos = item->pos(); const double oldRot = item->rotation();
    const QPointF newPos = oldPos + QPointF(15, -7); const double newRot = oldRot + 12.5;
    stack.push(new UndoCmd::TransformShapeCommand(item, oldPos, oldRot, newPos, newRot));
    QCOMPARE(item->pos(), newPos);
    QCOMPARE(item->rotation(), newRot);
    QVERIFY(item->model());
    QCOMPARE(item->model()->transform().m31(), newPos.x());
    QCOMPARE(item->model()->transform().m32(), newPos.y());
    QCOMPARE(item->model()->rotationDegrees(), newRot);
    stack.undo();
    QCOMPARE(item->pos(), oldPos);
    QCOMPARE(item->rotation(), oldRot);
}

void UndoCommandsTest::edit_json_and_undo() {
    DrawingScene scene; QUndoStack stack; scene.setUndoStack(&stack);
    auto* item = new ShapeItem(std::make_unique<Circle>(QPointF(0,0), 5.0));
    scene.addItem(item);
    auto oldJ = item->model()->ToJson(); oldJ["type"] = QStringLiteral("Circle");
    auto newJ = oldJ; auto g = newJ["geom"].toObject(); g["r"] = 12.0; newJ["geom"] = g;
    stack.push(new UndoCmd::EditShapeJsonCommand(item, oldJ, newJ));
    QVERIFY(std::abs(dynamic_cast<Circle*>(item->model())->radius() - 12.0) < 1e-9);
    stack.undo();
    QVERIFY(std::abs(dynamic_cast<Circle*>(item->model())->radius() - 5.0) < 1e-9);
}

void UndoCommandsTest::delete_and_undo() {
    DrawingScene scene; QUndoStack stack; scene.setUndoStack(&stack);
    auto* a = new ShapeItem(std::make_unique<Rectangle>(QRectF(0,0,10,10))); scene.addItem(a);
    auto* b = new ShapeItem(std::make_unique<Rectangle>(QRectF(20,0,10,10))); scene.addItem(b);
    a->setSelected(true); b->setSelected(true);
    // 预先保存 JSON 供撤销使用
    std::vector<QJsonObject> jsons;
    for (auto* it : scene.selectedItems()) {
        if (auto* si = dynamic_cast<ShapeItem*>(it)) { auto j = si->model()->ToJson(); j["type"] = si->typeName(); jsons.push_back(j); }
    }
    const int before = shapeItemCount(&scene);
    stack.push(new UndoCmd::DeleteShapesCommand(&scene, jsons));
    QCOMPARE(shapeItemCount(&scene), before - 2);
    stack.undo();
    QCOMPARE(shapeItemCount(&scene), before);
}

QTEST_MAIN(UndoCommandsTest)
#include "test_undo.moc"

