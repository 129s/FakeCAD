set(TARGET_NAME fakecad)

add_executable(${TARGET_NAME}
    WIN32
    main.cpp
    MainWindow.h
    MainWindow.cpp
    ui/ShapeItem.h
    ui/ShapeItem.cpp
    core/Shape.h
    core/Shape.cpp
    core/shapes/LineSegment.h
    core/shapes/LineSegment.cpp
    core/shapes/Rectangle.h
    core/shapes/Rectangle.cpp
    core/shapes/Circle.h
    core/shapes/Circle.cpp
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

