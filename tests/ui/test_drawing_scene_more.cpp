// UI：在 DrawingScene 中绘制圆与椭圆
#include <QtTest/QtTest>
#include "ui/CanvasView.h"
#include "ui/DrawingScene.h"
#include "ui/ShapeItem.h"

class DrawingSceneMoreTest : public QObject {
    Q_OBJECT
private slots:
    void draw_circle();
    void draw_ellipse();
};

void DrawingSceneMoreTest::draw_circle() {
    DrawingScene scene; scene.setSceneRect(0,0,400,300);
    CanvasView view(&scene);
    view.resize(320,240); view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    scene.setMode(DrawingScene::Mode::Circle);
    const QPoint startVp = view.mapFromScene(QPointF(100,100));
    const QPoint endVp   = view.mapFromScene(QPointF(150,100)); // 半径 50
    int before = 0; { auto items = scene.items(); for (auto* it : items) if (dynamic_cast<ShapeItem*>(it)) ++before; }
    QTest::mousePress(view.viewport(), Qt::LeftButton, Qt::NoModifier, startVp);
    QTest::mouseMove(view.viewport(), endVp);
    QTest::mouseRelease(view.viewport(), Qt::LeftButton, Qt::NoModifier, endVp);
    int after = 0; { auto items = scene.items(); for (auto* it : items) if (dynamic_cast<ShapeItem*>(it)) ++after; }
    QVERIFY(after >= before + 1);
}

void DrawingSceneMoreTest::draw_ellipse() {
    DrawingScene scene; scene.setSceneRect(0,0,400,300);
    CanvasView view(&scene);
    view.resize(320,240); view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    scene.setMode(DrawingScene::Mode::Ellipse);
    const QPoint startVp = view.mapFromScene(QPointF(200,150));
    const QPoint endVp   = view.mapFromScene(QPointF(260,180)); // rx=60, ry=30
    int before = 0; { auto items = scene.items(); for (auto* it : items) if (dynamic_cast<ShapeItem*>(it)) ++before; }
    QTest::mousePress(view.viewport(), Qt::LeftButton, Qt::NoModifier, startVp);
    QTest::mouseMove(view.viewport(), endVp);
    QTest::mouseRelease(view.viewport(), Qt::LeftButton, Qt::NoModifier, endVp);
    int after = 0; { auto items = scene.items(); for (auto* it : items) if (dynamic_cast<ShapeItem*>(it)) ++after; }
    QVERIFY(after >= before + 1);
}

QTEST_MAIN(DrawingSceneMoreTest)
#include "test_drawing_scene_more.moc"
