// UI 测试：控制点拖拽（几何编辑/旋转）在 RubberBandDrag 下仍应生效
#include <QtTest/QtTest>
#include <QtWidgets/QApplication>
#include <QUndoStack>
#include <QMouseEvent>

#include "ui/CanvasView.h"
#include "ui/DrawingScene.h"
#include "ui/ShapeItem.h"
#include "ui/ControlPointItem.h"

#include "core/shapes/Rectangle.h"
#include "core/shapes/Circle.h"

static ControlPointItem* findHandle(ShapeItem* item, ControlPointItem::Kind kind, int index) {
    for (auto* child : item->childItems()) {
        auto* h = dynamic_cast<ControlPointItem*>(child);
        if (!h) continue;
        if (h->kind() == kind && h->index() == index) return h;
    }
    return nullptr;
}

static QPoint toViewport(CanvasView& view, const QPointF& scenePos) {
    return view.mapFromScene(scenePos);
}

static void sendPress(CanvasView& view, const QPoint& pressVp) {
    auto* vp = view.viewport();
    const QPoint globalPress = vp->mapToGlobal(pressVp);
    {
        QMouseEvent ev(QEvent::MouseButtonPress, pressVp, globalPress, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QCoreApplication::sendEvent(vp, &ev);
    }
    QCoreApplication::processEvents();
}

static void sendMoveWithLeft(CanvasView& view, const QPoint& moveVp) {
    auto* vp = view.viewport();
    const QPoint globalMove = vp->mapToGlobal(moveVp);
    {
        QMouseEvent ev(QEvent::MouseMove, moveVp, globalMove, Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
        QCoreApplication::sendEvent(vp, &ev);
    }
    QCoreApplication::processEvents();
}

static void sendRelease(CanvasView& view, const QPoint& releaseVp) {
    auto* vp = view.viewport();
    const QPoint globalRelease = vp->mapToGlobal(releaseVp);
    {
        QMouseEvent ev(QEvent::MouseButtonRelease, releaseVp, globalRelease, Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
        QCoreApplication::sendEvent(vp, &ev);
    }
    QCoreApplication::processEvents();
}

class HandleInteractionTest : public QObject {
    Q_OBJECT
private slots:
    void rect_resize_updates_model_and_handles();
    void rotation_handle_changes_rotation();
};

void HandleInteractionTest::rect_resize_updates_model_and_handles() {
    DrawingScene scene;
    scene.setMode(DrawingScene::Mode::Rect); // 模拟用户仍处于绘制工具中
    scene.setSceneRect(-200, -200, 400, 400);
    CanvasView view(&scene);
    view.setDragMode(QGraphicsView::RubberBandDrag);
    view.resize(400, 400);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    auto* item = new ShapeItem(std::make_unique<Rectangle>(QRectF(0, 0, 100, 50)));
    scene.addItem(item);
    item->setSelected(true);
    QCoreApplication::processEvents();

    auto* tl = findHandle(item, ControlPointItem::Kind::Corner, 0);
    auto* tr = findHandle(item, ControlPointItem::Kind::Corner, 1);
    auto* br = findHandle(item, ControlPointItem::Kind::Corner, 2);
    auto* bl = findHandle(item, ControlPointItem::Kind::Corner, 3);
    QVERIFY(tl && tr && br && bl);

    // 取一个稍微偏离顶点的点：落在控制点方块内，但尽量避免与矩形边界的命中冲突
    const QPointF pressScene = tl->scenePos() + QPointF(-3, -3);
    const QPointF moveScene(20, 10); // 缩小：左上角向内移动
    sendPress(view, toViewport(view, pressScene));
    sendMoveWithLeft(view, toViewport(view, moveScene));

    auto* rc = dynamic_cast<Rectangle*>(item->model());
    QVERIFY(rc);
    const QRectF r = rc->rect().normalized();
    QCOMPARE(r.x(), 20.0);
    QCOMPARE(r.y(), 10.0);
    QCOMPARE(r.width(), 80.0);
    QCOMPARE(r.height(), 40.0);

    // 其它控制点应随几何更新而同步移动（拖拽中即可观察到）
    QCOMPARE(tr->pos(), QPointF(100, 10));
    QCOMPARE(br->pos(), QPointF(100, 50));
    QCOMPARE(bl->pos(), QPointF(20, 50));

    sendRelease(view, toViewport(view, moveScene));

    // 松手后 handles 会被重建，仍应与模型一致
    auto* tr2 = findHandle(item, ControlPointItem::Kind::Corner, 1);
    auto* br2 = findHandle(item, ControlPointItem::Kind::Corner, 2);
    auto* bl2 = findHandle(item, ControlPointItem::Kind::Corner, 3);
    QVERIFY(tr2 && br2 && bl2);
    QCOMPARE(tr2->pos(), QPointF(100, 10));
    QCOMPARE(br2->pos(), QPointF(100, 50));
    QCOMPARE(bl2->pos(), QPointF(20, 50));
}

void HandleInteractionTest::rotation_handle_changes_rotation() {
    DrawingScene scene;
    scene.setMode(DrawingScene::Mode::Circle); // 模拟用户仍处于绘制工具中
    scene.setSceneRect(-300, -300, 600, 600);
    auto* undo = new QUndoStack(&scene);
    scene.setUndoStack(undo);
    CanvasView view(&scene);
    view.setDragMode(QGraphicsView::RubberBandDrag);
    view.resize(500, 500);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    auto* item = new ShapeItem(std::make_unique<Circle>(QPointF(0, 0), 50));
    scene.addItem(item);
    item->setSelected(true);
    QCoreApplication::processEvents();

    auto* rot = findHandle(item, ControlPointItem::Kind::Rotation, 0);
    QVERIFY(rot);

    const QPointF pressScene = rot->scenePos();
    const QPointF moveScene(80, 0); // 相对中心从上移到右 -> 约 90°
    sendPress(view, toViewport(view, pressScene));
    sendMoveWithLeft(view, toViewport(view, moveScene));
    sendRelease(view, toViewport(view, moveScene));

    QVERIFY(std::abs(item->rotation() - 90.0) < 5.0);
    QVERIFY(item->model());
    QVERIFY(std::abs(item->model()->rotationDegrees() - 90.0) < 5.0);
}

QTEST_MAIN(HandleInteractionTest)
#include "test_handles.moc"
