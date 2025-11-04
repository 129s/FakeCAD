#pragma once

#include <QMainWindow>

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;
private:
    // actions
    QAction* actNew{};
    QAction* actOpen{};
    QAction* actSave{};
    QAction* actExit{};

    QAction* actZoomIn{};
    QAction* actZoomOut{};
    QAction* actAbout{};

    // ui builders
    void createActions();
    void createMenus();
    void createToolbars();
    void createStatusbar();

private slots:
    void onNew();
    void onOpen();
    void onSave();
    void onExit();
    void onAbout();
};
