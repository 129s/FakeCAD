// E2E 测试：主窗口打开后通过视图绘制一个图形
#include <QtTest/QtTest>
#include <QtWidgets/QApplication>

#include "MainWindow.h"
#include "ui/CanvasView.h"
#include "ui/DrawingScene.h"
#include "ui/ShapeItem.h"
#include "ui/ControlPointItem.h"

class E2ETest : public QObject {
    Q_OBJECT
private slots:
    void draw_rectangle_via_view();
    void close_after_handle_edit();
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

void E2ETest::close_after_handle_edit() {
    MainWindow w;
    w.resize(800,600);
    w.show();
    QVERIFY(QTest::qWaitForWindowExposed(&w));

    auto* view = w.findChild<CanvasView*>();
    auto* scene = w.findChild<DrawingScene*>();
    QVERIFY(view != nullptr);
    QVERIFY(scene != nullptr);

    // 选中一个矩形 ShapeItem
    ShapeItem* rectItem = nullptr;
    for (auto* it : scene->items()) {
        auto* si = dynamic_cast<ShapeItem*>(it);
        if (!si) continue;
        if (si->typeName() == QStringLiteral("Rectangle")) { rectItem = si; break; }
    }
    QVERIFY(rectItem != nullptr);
    rectItem->setSelected(true);
    QCoreApplication::processEvents();

    // 找一个角点控制点（index 0: topLeft）
    ControlPointItem* corner = nullptr;
    for (auto* child : rectItem->childItems()) {
        auto* h = dynamic_cast<ControlPointItem*>(child);
        if (!h) continue;
        if (h->kind() == ControlPointItem::Kind::Corner && h->index() == 0) { corner = h; break; }
    }
    QVERIFY(corner != nullptr);

    const QPointF pressScene = corner->scenePos() + QPointF(-3, -3);
    const QPointF moveScene(0, 0); // 向 (0,0) 拖一下，触发一次几何编辑

    const QPoint pressVp = view->mapFromScene(pressScene);
    const QPoint moveVp = view->mapFromScene(moveScene);
    QTest::mousePress(view->viewport(), Qt::LeftButton, Qt::NoModifier, pressVp);
    QTest::mouseMove(view->viewport(), moveVp);
    QTest::mouseRelease(view->viewport(), Qt::LeftButton, Qt::NoModifier, moveVp);
    QCoreApplication::processEvents();

    w.close();
    QTest::qWait(30);
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QCoreApplication::processEvents();
    QVERIFY(!w.isVisible());
}

QTEST_MAIN(E2ETest)
#include "test_drawing_flow.moc"
