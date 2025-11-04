#include "MainWindow.h"

#include <QAction>
#include <QApplication>
#include <QMenuBar>
#include <QStatusBar>
#include <QToolBar>
#include <QMessageBox>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    setWindowTitle(QStringLiteral("FakeCAD"));

    createActions();
    createMenus();
    createToolbars();
    createStatusbar();
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
void MainWindow::onOpen() { statusBar()->showMessage(tr("打开文件（待实现）"), 2000); }
void MainWindow::onSave() { statusBar()->showMessage(tr("保存文件（待实现）"), 2000); }
void MainWindow::onExit() { QApplication::quit(); }
void MainWindow::onAbout() {
    QMessageBox::about(this, tr("关于 FakeCAD"), tr("FakeCAD\nC++17 / CMake / Qt6"));
}
