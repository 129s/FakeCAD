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

# ---------------------------
# 安装规则
# 将可执行文件安装到前缀根目录（便于 windeployqt 就地部署依赖）
# 同时在 Debug/RelWithDebInfo 下安装 PDB（如存在）。
# ---------------------------
install(TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION .
    BUNDLE  DESTINATION .
    LIBRARY DESTINATION .
    ARCHIVE DESTINATION .
)

# 安装 PDB（可选）
if (MSVC)
    install(FILES $<TARGET_PDB_FILE:${TARGET_NAME}> DESTINATION . OPTIONAL)
endif()
