cmake_minimum_required(VERSION 3.20)

project(FakeCAD VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 构建选项：单EXE（静态链接）与瘦身优化
option(FAKECAD_SINGLE_EXE "构建单EXE（需要静态Qt/静态运行库）" OFF)
option(FAKECAD_SIZE_OPTIMIZE "启用二进制瘦身优化（LTO/GC sections/strip 等）" OFF)

# LTO/IPO：尽量在 Release 开启
if (FAKECAD_SIZE_OPTIMIZE)
    # CMake 3.9+ 支持 IPO；3.20 最低版本满足
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL ON)
endif()

find_package(Qt6 6.4 REQUIRED COMPONENTS Widgets Gui Core)

add_subdirectory(src)

# 测试集（按需开启）
option(FAKECAD_BUILD_TESTS "是否构建测试" OFF)
if (FAKECAD_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ---------------------------
# CPack（用于生成ZIP包的基础配置）
# 注意：Qt 运行时依赖在 Windows 上通过 windeployqt 由脚本完成，
# CPack 只负责将安装产物打包成 ZIP。
# ---------------------------
set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_NAME "FakeCAD")
set(CPACK_PACKAGE_VENDOR "FakeCAD")
set(CPACK_PACKAGE_CONTACT "")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DIRECTORY "${CMAKE_BINARY_DIR}/_packages")

# 生成的 ZIP 文件名（不含配置后缀，配置由脚本区分）
set(CPACK_PACKAGE_FILE_NAME "FakeCAD-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}")

include(CPack)
