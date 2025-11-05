// E2E 测试：主窗口打开后通过视图绘制一个图形
#include <QtTest/QtTest>
#include <QtWidgets/QApplication>

#include "MainWindow.h"
#include "ui/CanvasView.h"
#include "ui/DrawingScene.h"
#include "ui/ShapeItem.h"

class E2ETest : public QObject {
    Q_OBJECT
private slots:
    void draw_rectangle_via_view();
};

void E2ETest::draw_rectangle_via_view() {
    MainWindow w;
    w.resize(800,600);
    w.show();
    QVERIFY(QTest::qWaitForWindowExposed(&w));

    auto* view = w.findChild<CanvasView*>();
    auto* scene = w.findChild<DrawingScene*>();
    QVERIFY(view != nullptr);
    QVERIFY(scene != nullptr);

    // 初始存在 3 个示例元素
    int before = 0; for (auto* it : scene->items()) if (dynamic_cast<ShapeItem*>(it)) ++before;

    scene->setMode(DrawingScene::Mode::Rect);
    const QPoint startVp = view->mapFromScene(QPointF(50,50));
    const QPoint endVp   = view->mapFromScene(QPointF(150,120));
    QTest::mousePress(view->viewport(), Qt::LeftButton, Qt::NoModifier, startVp);
    QTest::mouseMove(view->viewport(), endVp);
    QTest::mouseRelease(view->viewport(), Qt::LeftButton, Qt::NoModifier, endVp);

    // 应增加一个 ShapeItem
    int after = 0; for (auto* it : scene->items()) if (dynamic_cast<ShapeItem*>(it)) ++after;
    QVERIFY(after >= before + 1);
}

QTEST_MAIN(E2ETest)
#include "test_drawing_flow.moc"

