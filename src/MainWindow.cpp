#include "MainWindow.h"

#include <QAction>
#include <QApplication>
#include <QMenuBar>
#include <QStatusBar>
#include <QToolBar>
#include <QActionGroup>
#include <QMessageBox>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QFileDialog>
#include <QDockWidget>
#include <QShortcut>
#include <QUndoStack>
#include <memory>

#include "ui/ShapeItem.h"
#include "ui/DrawingScene.h"
#include "ui/CanvasView.h"
#include "ui/PropertyPanel.h"
#include "core/shapes/LineSegment.h"
#include "core/shapes/Rectangle.h"
#include "core/shapes/Circle.h"
#include "core/Serialization.h"
#include "undo/Commands.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    setWindowTitle(QStringLiteral("FakeCAD"));

    // 先创建撤销栈，菜单中会用到
    undo_ = new QUndoStack(this);

    createActions();
    createMenus();
    createToolbars();
    createStatusbar();

    // 先有 scene/view，再创建依赖其信号的面板/停靠栏
    scene = new DrawingScene(this);
    scene->setSceneRect(-5000, -5000, 10000, 10000);
    scene->setUndoStack(undo_);

    view = new CanvasView(scene, this);
    view->setDragMode(QGraphicsView::RubberBandDrag);
    connect(view, &CanvasView::mouseScenePosChanged, this, [this](const QPointF& p){
        statusBar()->showMessage(tr("坐标: (%1, %2)").arg(p.x(), 0, 'f', 1).arg(p.y(), 0, 'f', 1));
    });
    setCentralWidget(view);

    createPropertyDock();

    // 示例元素（基于模型的适配器）
    scene->addItem(new ShapeItem(std::make_unique<LineSegment>(QPointF(-100, -80), QPointF(150, 60))));
    scene->addItem(new ShapeItem(std::make_unique<Rectangle>(QRectF(-200, -150, 120, 90))));
    scene->addItem(new ShapeItem(std::make_unique<Circle>(QPointF(180, -40), 60)));
}

void MainWindow::createActions() {
    actNew = new QAction(tr("新建"), this);
    actOpen = new QAction(tr("打开..."), this);
    actSave = new QAction(tr("保存"), this);
    actExit = new QAction(tr("退出"), this);

    actNew->setShortcut(QKeySequence::New);
    actOpen->setShortcut(QKeySequence::Open);
    actSave->setShortcut(QKeySequence::Save);
    actExit->setShortcut(QKeySequence::Quit);

    connect(actNew, &QAction::triggered, this, &MainWindow::onNew);
    connect(actOpen, &QAction::triggered, this, &MainWindow::onOpen);
    connect(actSave, &QAction::triggered, this, &MainWindow::onSave);
    connect(actExit, &QAction::triggered, this, &MainWindow::onExit);

    actZoomIn = new QAction(tr("放大"), this);
    actZoomOut = new QAction(tr("缩小"), this);
    actResetZoom = new QAction(tr("重置缩放"), this);
    actZoomIn->setShortcut(QKeySequence::ZoomIn);
    actZoomOut->setShortcut(QKeySequence::ZoomOut);
    actResetZoom->setShortcut(QKeySequence(tr("Ctrl+0")));
    connect(actZoomIn, &QAction::triggered, this, &MainWindow::onZoomIn);
    connect(actZoomOut, &QAction::triggered, this, &MainWindow::onZoomOut);
    connect(actResetZoom, &QAction::triggered, this, &MainWindow::onResetZoom);
    // 暂留快捷键，等画布接入后启用

    actAbout = new QAction(tr("关于"), this);
    connect(actAbout, &QAction::triggered, this, &MainWindow::onAbout);

    // 绘制工具
    actSelect = new QAction(tr("选择"), this);
    actDrawLine = new QAction(tr("线段"), this);
    actDrawRect = new QAction(tr("矩形"), this);
    actDrawCircle = new QAction(tr("圆"), this);
    actDrawEllipse = new QAction(tr("椭圆"), this);

    actSelect->setCheckable(true);
    actDrawLine->setCheckable(true);
    actDrawRect->setCheckable(true);
    actDrawCircle->setCheckable(true);
    actDrawEllipse->setCheckable(true);
    actSelect->setShortcut(QKeySequence(tr("1")));
    actDrawLine->setShortcut(QKeySequence(tr("2")));
    actDrawRect->setShortcut(QKeySequence(tr("3")));
    actDrawCircle->setShortcut(QKeySequence(tr("4")));
    actDrawEllipse->setShortcut(QKeySequence(tr("5")));

    drawGroup = new QActionGroup(this);
    drawGroup->setExclusive(true);
    drawGroup->addAction(actSelect);
    drawGroup->addAction(actDrawLine);
    drawGroup->addAction(actDrawRect);
    drawGroup->addAction(actDrawCircle);
    drawGroup->addAction(actDrawEllipse);

    actSelect->setChecked(true);

    connect(actSelect, &QAction::toggled, this, &MainWindow::onSelectToggled);
    connect(actDrawLine, &QAction::toggled, this, &MainWindow::onDrawLineToggled);
    connect(actDrawRect, &QAction::toggled, this, &MainWindow::onDrawRectToggled);
    connect(actDrawCircle, &QAction::toggled, this, &MainWindow::onDrawCircleToggled);
    connect(actDrawEllipse, &QAction::toggled, this, &MainWindow::onDrawEllipseToggled);

    // Esc 返回选择
    auto escShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(escShortcut, &QShortcut::activated, this, [this]{ actSelect->setChecked(true); });

    // 视图选项
    actToggleGrid = new QAction(tr("显示网格"), this);
    actToggleGrid->setCheckable(true);
    actToggleGrid->setChecked(true);
    actToggleGrid->setShortcut(QKeySequence(tr("G")));
    connect(actToggleGrid, &QAction::toggled, this, [this](bool on){ scene->setShowGrid(on); });

    actSnapGrid = new QAction(tr("吸附到网格"), this);
    actSnapGrid->setCheckable(true);
    actSnapGrid->setChecked(false);
    actSnapGrid->setShortcut(QKeySequence(tr("Shift+G")));
    connect(actSnapGrid, &QAction::toggled, this, [this](bool on){ scene->setSnapToGrid(on); });

    // 删除选中
    actDelete = new QAction(tr("删除选中"), this);
    actDelete->setShortcut(QKeySequence::Delete);
    connect(actDelete, &QAction::triggered, this, &MainWindow::onDelete);
}

void MainWindow::createMenus() {
    auto fileMenu = menuBar()->addMenu(tr("文件"));
    fileMenu->addAction(actNew);
    fileMenu->addAction(actOpen);
    fileMenu->addAction(actSave);
    fileMenu->addSeparator();
    fileMenu->addAction(actExit);

    auto editMenu = menuBar()->addMenu(tr("编辑"));
    auto undoAct = undo_->createUndoAction(this, tr("撤销"));
    undoAct->setShortcut(QKeySequence::Undo);
    auto redoAct = undo_->createRedoAction(this, tr("重做"));
    redoAct->setShortcuts({QKeySequence::Redo, QKeySequence(tr("Ctrl+Y"))});
    editMenu->addAction(undoAct);
    editMenu->addAction(redoAct);
    editMenu->addSeparator();
    editMenu->addAction(actDelete);

    auto viewMenu = menuBar()->addMenu(tr("视图"));
    viewMenu->addAction(actZoomIn);
    viewMenu->addAction(actZoomOut);
    viewMenu->addAction(actResetZoom);
    viewMenu->addSeparator();
    viewMenu->addAction(actToggleGrid);
    viewMenu->addAction(actSnapGrid);

    auto helpMenu = menuBar()->addMenu(tr("帮助"));
    helpMenu->addAction(actAbout);
}

void MainWindow::createToolbars() {
    auto fileBar = addToolBar(tr("文件"));
    fileBar->addAction(actNew);
    fileBar->addAction(actOpen);
    fileBar->addAction(actSave);

    auto viewBar = addToolBar(tr("视图"));
    viewBar->addAction(actZoomIn);
    viewBar->addAction(actZoomOut);
    viewBar->addAction(actResetZoom);
    viewBar->addAction(actToggleGrid);
    viewBar->addAction(actSnapGrid);

    auto drawBar = addToolBar(tr("绘制"));
    drawBar->addAction(actSelect);
    drawBar->addAction(actDrawLine);
    drawBar->addAction(actDrawRect);
    drawBar->addAction(actDrawCircle);
    drawBar->addAction(actDrawEllipse);
}

void MainWindow::createStatusbar() {
    statusBar()->showMessage(tr("就绪"));
}

void MainWindow::createPropertyDock() {
    propPanel = new PropertyPanel(this);
    propDock = new QDockWidget(tr("属性"), this);
    propDock->setWidget(propPanel);
    addDockWidget(Qt::RightDockWidgetArea, propDock);
    // 依赖 scene 已创建
    connect(scene, &QGraphicsScene::selectionChanged, this, &MainWindow::onSelectionChanged);
}

void MainWindow::onNew() { statusBar()->showMessage(tr("新建工程（待实现）"), 2000); }
void MainWindow::onSave() {
    const auto path = QFileDialog::getSaveFileName(this, tr("保存为"), QString(), tr("FakeCAD JSON (*.json)"));
    if (path.isEmpty()) return;
    std::vector<Shape*> shapes;
    for (auto* item : scene->items()) {
        if (auto* si = dynamic_cast<ShapeItem*>(item)) {
            // 同步项位置到模型（仅平移）
            const auto p = si->pos();
            si->model()->MoveTo(p.x(), p.y());
            si->model()->setRotationDegrees(si->rotation());
            shapes.push_back(si->model());
        }
    }
    QString err;
    if (Ser::SaveToFile(path, shapes, &err)) {
        statusBar()->showMessage(tr("已保存: %1").arg(path), 3000);
    } else {
        QMessageBox::warning(this, tr("保存失败"), err);
    }
}
void MainWindow::onExit() { QApplication::quit(); }
void MainWindow::onAbout() {
    QMessageBox::about(this, tr("关于 FakeCAD"), tr("FakeCAD\nC++17 / CMake / Qt6"));
}
void MainWindow::onOpen() {
    const auto path = QFileDialog::getOpenFileName(this, tr("打开"), QString(), tr("FakeCAD JSON (*.json)"));
    if (path.isEmpty()) return;
    QString err;
    propPanel->clearTarget();
    auto shapes = Ser::LoadFromFile(path, &err);
    if (!err.isEmpty()) {
        QMessageBox::warning(this, tr("打开失败"), err);
        return;
    }
    scene->clear();
    for (auto& sp : shapes) {
        scene->addItem(new ShapeItem(std::move(sp)));
    }
    statusBar()->showMessage(tr("已加载: %1").arg(path), 3000);
}

void MainWindow::updateViewDragMode() {
    auto dm = scene->mode();
    if (dm == DrawingScene::Mode::None) {
        view->setDragMode(QGraphicsView::RubberBandDrag);
        statusBar()->showMessage(tr("选择模式"), 2000);
    } else {
        view->setDragMode(QGraphicsView::NoDrag);
        switch (dm) {
        case DrawingScene::Mode::Line:   statusBar()->showMessage(tr("绘制线段：按下拖拽释放"), 2000); break;
        case DrawingScene::Mode::Rect:   statusBar()->showMessage(tr("绘制矩形：按下拖拽释放"), 2000); break;
        case DrawingScene::Mode::Circle: statusBar()->showMessage(tr("绘制圆（中心+半径）：按下拖拽释放"), 2000); break;
        case DrawingScene::Mode::Ellipse: statusBar()->showMessage(tr("绘制椭圆（中心+半径X/Y）：按下拖拽释放"), 2000); break;
        default: break;
        }
    }
}

void MainWindow::onSelectToggled(bool on) {
    if (!on) return;
    scene->setMode(DrawingScene::Mode::None);
    updateViewDragMode();
}

void MainWindow::onDrawLineToggled(bool on) {
    if (!on) return;
    scene->setMode(DrawingScene::Mode::Line);
    updateViewDragMode();
}

void MainWindow::onDrawRectToggled(bool on) {
    if (!on) return;
    scene->setMode(DrawingScene::Mode::Rect);
    updateViewDragMode();
}

void MainWindow::onDrawCircleToggled(bool on) {
    if (!on) return;
    scene->setMode(DrawingScene::Mode::Circle);
    updateViewDragMode();
}
void MainWindow::onDrawEllipseToggled(bool on) {
    if (!on) return;
    scene->setMode(DrawingScene::Mode::Ellipse);
    updateViewDragMode();
}

void MainWindow::onZoomIn() { view->zoomBy(1.15); }
void MainWindow::onZoomOut() { view->zoomBy(1.0/1.15); }
void MainWindow::onResetZoom() { view->resetZoom(); }

void MainWindow::onDelete() {
    propPanel->clearTarget();
    // 收集 JSON 快照
    std::vector<QJsonObject> snap;
    for (auto* it : scene->selectedItems()) {
        if (auto* si = dynamic_cast<ShapeItem*>(it)) {
            snap.push_back(si->model()->ToJson());
        }
    }
    if (!snap.empty() && undo_) {
        undo_->push(new UndoCmd::DeleteShapesCommand(scene, snap));
    }
}

void MainWindow::onSelectionChanged() {
    auto sel = scene->selectedItems();
    if (sel.isEmpty()) { propPanel->clearTarget(); return; }
    for (auto* it : sel) {
        if (auto* si = dynamic_cast<ShapeItem*>(it)) { propPanel->setShapeItem(si); return; }
    }
    propPanel->clearTarget();
}
