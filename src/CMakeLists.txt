set(TARGET_NAME fakecad)

add_executable(${TARGET_NAME}
    WIN32
    main.cpp
    MainWindow.h
    MainWindow.cpp
    ui/ShapeItem.h
    ui/ShapeItem.cpp
    ui/DrawingScene.h
    ui/DrawingScene.cpp
    ui/CanvasView.h
    ui/CanvasView.cpp
    ui/ControlPointItem.h
    ui/ControlPointItem.cpp
    ui/PropertyPanel.h
    ui/PropertyPanel.cpp
    core/Shape.h
    core/Shape.cpp
    core/Serialization.h
    core/Serialization.cpp
    undo/Commands.h
    undo/Commands.cpp
    core/shapes/LineSegment.h
    core/shapes/LineSegment.cpp
    core/shapes/Rectangle.h
    core/shapes/Rectangle.cpp
    core/shapes/Circle.h
    core/shapes/Circle.cpp
    core/shapes/Triangle.h
    core/shapes/Triangle.cpp
    core/shapes/Polygon.h
    core/shapes/Polygon.cpp
    core/shapes/Polyline.h
    core/shapes/Polyline.cpp
    core/shapes/Ellipse.h
    core/shapes/Ellipse.cpp
)

target_link_libraries(${TARGET_NAME}
    PRIVATE
        Qt6::Widgets
        Qt6::Gui
        Qt6::Core
)

target_include_directories(${TARGET_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

if (MSVC)
    target_compile_options(${TARGET_NAME} PRIVATE /W4)
else()
    target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_property(TARGET ${TARGET_NAME} PROPERTY VS_DPI_AWARE "PerMonitorV2")
