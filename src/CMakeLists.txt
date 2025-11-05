set(APP_TARGET fakecad)

# 将绝大部分源码编译为可复用静态库，便于测试链接
add_library(fakecad_lib STATIC
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

target_link_libraries(fakecad_lib
    PUBLIC
        Qt6::Widgets
        Qt6::Gui
        Qt6::Core
)

target_include_directories(fakecad_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

if (MSVC)
    target_compile_options(fakecad_lib PRIVATE /W4)
else()
    target_compile_options(fakecad_lib PRIVATE -Wall -Wextra -Wpedantic)
endif()

# 应用程序仅包含入口 main 并链接库
add_executable(${APP_TARGET} WIN32
    main.cpp
)

target_link_libraries(${APP_TARGET}
    PRIVATE
        fakecad_lib
)

set_property(TARGET ${APP_TARGET} PROPERTY VS_DPI_AWARE "PerMonitorV2")

# ---------------------------
# 安装规则
# 将可执行文件安装到前缀根目录（便于 windeployqt 就地部署依赖）
# ---------------------------
install(TARGETS ${APP_TARGET}
    RUNTIME DESTINATION .
    BUNDLE  DESTINATION .
    LIBRARY DESTINATION .
    ARCHIVE DESTINATION .
)

# 安装 PDB（可选）
if (MSVC)
    install(FILES $<TARGET_PDB_FILE:${APP_TARGET}> DESTINATION . OPTIONAL)
endif()
