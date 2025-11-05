cmake_minimum_required(VERSION 3.20)

# 公共包含路径（暴露 src 头以便测试直接 include）
include(GNUInstallDirs)

# 单元测试（自研极简测试框架，零依赖）
add_executable(unit_tests
    unit/test_shapes.cpp
    common/minitest.h
)
target_include_directories(unit_tests PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/common)
target_link_libraries(unit_tests PRIVATE Qt6::Core Qt6::Gui fakecad_lib)
add_test(NAME unit COMMAND unit_tests)
set_tests_properties(unit PROPERTIES LABELS "unit")

# 集成测试（序列化/反序列化/文件 I/O）
add_executable(integration_tests
    integration/test_serialization.cpp
    common/minitest.h
)
target_include_directories(integration_tests PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/common)
target_link_libraries(integration_tests PRIVATE Qt6::Core Qt6::Gui fakecad_lib)
add_test(NAME integration COMMAND integration_tests)
set_tests_properties(integration PROPERTIES LABELS "integration")

# UI 测试（Qt Test）
find_package(Qt6 COMPONENTS Test Widgets Gui Core REQUIRED)

add_executable(ui_tests
    ui/test_canvasview.cpp
)
target_include_directories(ui_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(ui_tests PRIVATE Qt6::Test Qt6::Widgets Qt6::Gui Qt6::Core fakecad_lib)
add_test(NAME ui COMMAND ui_tests)
set_tests_properties(ui PROPERTIES LABELS "ui")

# 额外 UI 测试：圆/椭圆绘制
add_executable(ui_tests_scene
    ui/test_drawing_scene_more.cpp
)
target_include_directories(ui_tests_scene PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(ui_tests_scene PRIVATE Qt6::Test Qt6::Widgets Qt6::Gui Qt6::Core fakecad_lib)
add_test(NAME ui_scene COMMAND ui_tests_scene)
set_tests_properties(ui_scene PROPERTIES LABELS "ui")

# 撤销/重做命令测试
add_executable(ui_tests_undo
    ui/test_undo.cpp
)
target_include_directories(ui_tests_undo PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(ui_tests_undo PRIVATE Qt6::Test Qt6::Widgets Qt6::Gui Qt6::Core fakecad_lib)
add_test(NAME ui_undo COMMAND ui_tests_undo)
set_tests_properties(ui_undo PROPERTIES LABELS "ui")

# 扩展的集成测试：ApplyJsonToShape 覆盖所有形状
add_executable(integration_applyjson
    integration/test_applyjson.cpp
    common/minitest.h
)
target_include_directories(integration_applyjson PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/common)
target_link_libraries(integration_applyjson PRIVATE Qt6::Core Qt6::Gui fakecad_lib)
add_test(NAME integration_applyjson COMMAND integration_applyjson)
set_tests_properties(integration_applyjson PROPERTIES LABELS "integration")
# E2E 测试（构建完整主窗体并通过视图模拟操作）
add_executable(e2e_tests
    e2e/test_drawing_flow.cpp
)
target_include_directories(e2e_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(e2e_tests PRIVATE Qt6::Test Qt6::Widgets Qt6::Gui Qt6::Core fakecad_lib)
add_test(NAME e2e COMMAND e2e_tests)
set_tests_properties(e2e PROPERTIES LABELS "e2e")
