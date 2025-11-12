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

# 在 VS 生成器下声明 DPI 感知（布尔值兼容所有受支持版本）
if (MSVC)
    set_property(TARGET ${APP_TARGET} PROPERTY VS_DPI_AWARE ON)
endif()

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

# ---------------------------
# 单EXE（静态链接）与瘦身优化
# ---------------------------
if (FAKECAD_SINGLE_EXE)
    # 对应用目标启用单EXE相关定义
    target_compile_definitions(${APP_TARGET} PRIVATE FAKECAD_SINGLE_EXE)

    if (MSVC)
        # 使用静态运行库 /MT 和 /MTd（CMake 3.15+）
        set_property(TARGET fakecad_lib PROPERTY  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        set_property(TARGET ${APP_TARGET} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        # 链接：移除增量，启用函数合并/无用剔除
        target_link_options(${APP_TARGET} PRIVATE 
            $<$<CONFIG:Release>:/OPT:REF /OPT:ICF /INCREMENTAL:NO>
        )
    else()
        # MinGW/Clang/GCC（Windows 优先）：尽量静态链接运行库
        target_link_options(${APP_TARGET} PRIVATE -static -static-libstdc++ -static-libgcc)
    endif()

    # 尝试导入/链接静态平台插件（Windows）
    if (WIN32)
        if (COMMAND qt_import_plugins AND TARGET Qt6::QWindowsIntegrationPlugin)
            qt_import_plugins(${APP_TARGET} INCLUDE Qt6::QWindowsIntegrationPlugin)
        elseif (TARGET Qt6::QWindowsIntegrationPlugin)
            target_link_libraries(${APP_TARGET} PRIVATE Qt6::QWindowsIntegrationPlugin)
        endif()
    endif()
endif()

if (FAKECAD_SIZE_OPTIMIZE)
    if (MSVC)
        # 优化与链接时函数合并
        target_compile_options(fakecad_lib PRIVATE $<$<CONFIG:Release>:/O2 /GL>)
        target_compile_options(${APP_TARGET} PRIVATE  $<$<CONFIG:Release>:/O2 /GL>)
        target_link_options(${APP_TARGET} PRIVATE     $<$<CONFIG:Release>:/OPT:REF /OPT:ICF /INCREMENTAL:NO>)
    else()
        # 函数/数据节 + 链接时无用剔除 + 去符号（Release）
        target_compile_options(fakecad_lib PRIVATE $<$<CONFIG:Release>:-O2 -ffunction-sections -fdata-sections -DNDEBUG>)
        target_compile_options(${APP_TARGET} PRIVATE  $<$<CONFIG:Release>:-O2 -ffunction-sections -fdata-sections -DNDEBUG>)
        target_link_options(${APP_TARGET} PRIVATE     $<$<CONFIG:Release>:-Wl,--gc-sections -s>)
    endif()
endif()
