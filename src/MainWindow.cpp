#include "MainWindow.h"

#include <QAction>
#include <QApplication>
#include <QMenuBar>
#include <QStatusBar>
#include <QToolBar>
#include <QMessageBox>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QFileDialog>
#include <QDockWidget>
#include <memory>

#include "ui/ShapeItem.h"
#include "ui/DrawingScene.h"
#include "ui/PropertyPanel.h"
#include "core/shapes/LineSegment.h"
#include "core/shapes/Rectangle.h"
#include "core/shapes/Circle.h"
#include "core/Serialization.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    setWindowTitle(QStringLiteral("FakeCAD"));

    createActions();
    createMenus();
    createToolbars();
    createStatusbar();
    createPropertyDock();

    scene = new DrawingScene(this);
    scene->setSceneRect(-5000, -5000, 10000, 10000);

    view = new QGraphicsView(scene, this);
    view->setRenderHint(QPainter::Antialiasing, true);
    view->setDragMode(QGraphicsView::RubberBandDrag);
    setCentralWidget(view);

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
    // 暂留快捷键，等画布接入后启用

    actAbout = new QAction(tr("关于"), this);
    connect(actAbout, &QAction::triggered, this, &MainWindow::onAbout);

    // 绘制工具
    actSelect = new QAction(tr("选择"), this);
    actDrawLine = new QAction(tr("线段"), this);
    actDrawRect = new QAction(tr("矩形"), this);
    actDrawCircle = new QAction(tr("圆"), this);

    actSelect->setCheckable(true);
    actDrawLine->setCheckable(true);
    actDrawRect->setCheckable(true);
    actDrawCircle->setCheckable(true);

    drawGroup = new QActionGroup(this);
    drawGroup->setExclusive(true);
    drawGroup->addAction(actSelect);
    drawGroup->addAction(actDrawLine);
    drawGroup->addAction(actDrawRect);
    drawGroup->addAction(actDrawCircle);

    actSelect->setChecked(true);

    connect(actSelect, &QAction::toggled, this, &MainWindow::onSelectToggled);
    connect(actDrawLine, &QAction::toggled, this, &MainWindow::onDrawLineToggled);
    connect(actDrawRect, &QAction::toggled, this, &MainWindow::onDrawRectToggled);
    connect(actDrawCircle, &QAction::toggled, this, &MainWindow::onDrawCircleToggled);
}

void MainWindow::createMenus() {
    auto fileMenu = menuBar()->addMenu(tr("文件"));
    fileMenu->addAction(actNew);
    fileMenu->addAction(actOpen);
    fileMenu->addAction(actSave);
    fileMenu->addSeparator();
    fileMenu->addAction(actExit);

    auto viewMenu = menuBar()->addMenu(tr("视图"));
    viewMenu->addAction(actZoomIn);
    viewMenu->addAction(actZoomOut);

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

    auto drawBar = addToolBar(tr("绘制"));
    drawBar->addAction(actSelect);
    drawBar->addAction(actDrawLine);
    drawBar->addAction(actDrawRect);
    drawBar->addAction(actDrawCircle);
}

void MainWindow::createStatusbar() {
    statusBar()->showMessage(tr("就绪"));
}

void MainWindow::createPropertyDock() {
    propPanel = new PropertyPanel(this);
    propDock = new QDockWidget(tr("属性"), this);
    propDock->setWidget(propPanel);
    addDockWidget(Qt::RightDockWidgetArea, propDock);
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
            si->shape()->MoveTo(p.x(), p.y());
            shapes.push_back(si->shape());
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

void MainWindow::onSelectionChanged() {
    auto sel = scene->selectedItems();
    if (sel.isEmpty()) { propPanel->clearTarget(); return; }
    for (auto* it : sel) {
        if (auto* si = dynamic_cast<ShapeItem*>(it)) { propPanel->setShapeItem(si); return; }
    }
    propPanel->clearTarget();
}
