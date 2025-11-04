#pragma once

#include <QMainWindow>

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;
private:
    // 画布
    class CanvasView* view{};
    class DrawingScene* scene{};
private:
    // actions
    QAction* actNew{};
    QAction* actOpen{};
    QAction* actSave{};
    QAction* actExit{};

    QAction* actZoomIn{};
    QAction* actZoomOut{};
    QAction* actResetZoom{};
    QAction* actSelect{};
    QAction* actDrawLine{};
    QAction* actDrawRect{};
    QAction* actDrawCircle{};
    class QActionGroup* drawGroup{};
    QAction* actToggleGrid{};
    QAction* actSnapGrid{};
    QAction* actDelete{};
    QAction* actAbout{};

    // ui builders
    void createActions();
    void createMenus();
    void createToolbars();
    void createStatusbar();
    void createPropertyDock();
    void updateViewDragMode();

private slots:
    void onNew();
    void onOpen();
    void onSave();
    void onExit();
    void onAbout();
    void onSelectToggled(bool on);
    void onDrawLineToggled(bool on);
    void onDrawRectToggled(bool on);
    void onDrawCircleToggled(bool on);
    void onZoomIn();
    void onZoomOut();
    void onResetZoom();
    void onDelete();
    void onSelectionChanged();

private:
    class PropertyPanel* propPanel{};
    class QDockWidget* propDock{};
};
