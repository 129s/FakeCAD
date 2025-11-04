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
#include <memory>

#include "ui/ShapeItem.h"
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

    scene = new QGraphicsScene(this);
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
}

void MainWindow::createStatusbar() {
    statusBar()->showMessage(tr("就绪"));
}

void MainWindow::onNew() { statusBar()->showMessage(tr("新建工程（待实现）"), 2000); }
void MainWindow::onSave() {
    const auto path = QFileDialog::getSaveFileName(this, tr("保存为"), QString(), tr("FakeCAD JSON (*.json)"));
    if (path.isEmpty()) return;
    std::vector<Shape*> shapes;
    for (auto* item : scene->items()) {
        if (auto* si = dynamic_cast<ShapeItem*>(item)) shapes.push_back(si->shape());
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
        if (dynamic_cast<LineSegment*>(sp.get()) || dynamic_cast<Rectangle*>(sp.get()) || dynamic_cast<Circle*>(sp.get())) {
            scene->addItem(new ShapeItem(std::move(sp)));
        }
    }
    statusBar()->showMessage(tr("已加载: %1").arg(path), 3000);
}
