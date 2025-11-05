// UI 测试：CanvasView 与 DrawingScene 的基本行为
#include <QtTest/QtTest>
#include <QtWidgets/QApplication>

#include "ui/CanvasView.h"
#include "ui/DrawingScene.h"
#include "ui/ShapeItem.h"

class CanvasViewTest : public QObject {
    Q_OBJECT
private slots:
    void zoom_clamps();
    void draw_line_creates_item();
    void snap_to_grid();
};

void CanvasViewTest::zoom_clamps() {
    DrawingScene scene; scene.setSceneRect(0,0,200,200);
    CanvasView view(&scene);
    view.resize(300,300);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    // 放大到极限
    for (int i=0;i<50;++i) view.zoomBy(1.5);
    const qreal maxScale = view.transform().m11();
    QVERIFY(maxScale <= 10.0001);

    // 再缩小到极限
    for (int i=0;i<100;++i) view.zoomBy(1.0/1.5);
    const qreal minScale = view.transform().m11();
    QVERIFY(minScale >= 0.099);
}

void CanvasViewTest::draw_line_creates_item() {
    DrawingScene scene; scene.setSceneRect(0,0,200,200);
    CanvasView view(&scene);
    view.resize(300,300);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    scene.setMode(DrawingScene::Mode::Line);
    const QPoint startVp = view.mapFromScene(QPointF(10,10));
    const QPoint endVp   = view.mapFromScene(QPointF(100,60));

    const auto itemsBefore = scene.items().size();
    QTest::mousePress(view.viewport(), Qt::LeftButton, Qt::NoModifier, startVp);
    QTest::mouseMove(view.viewport(), endVp);
    QTest::mouseRelease(view.viewport(), Qt::LeftButton, Qt::NoModifier, endVp);

    // 只统计 ShapeItem 数量
    int shapeCount = 0;
    for (auto* it : scene.items()) if (dynamic_cast<ShapeItem*>(it)) ++shapeCount;
    QVERIFY(shapeCount >= 1);
    QVERIFY(scene.items().size() >= itemsBefore);
}

void CanvasViewTest::snap_to_grid() {
    DrawingScene scene;
    scene.setSnapToGrid(true);
    scene.setGridSize(10.0);
    QPointF p = scene.snapPoint(QPointF(13.2, 27.9));
    QCOMPARE(p.x(), 10.0);
    QCOMPARE(p.y(), 30.0);
}

QTEST_MAIN(CanvasViewTest)
#include "test_canvasview.moc"

