set(TARGET_NAME fakecad)

add_executable(${TARGET_NAME}
    WIN32
    main.cpp
    MainWindow.h
    MainWindow.cpp
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

