cmake_minimum_required(VERSION 3.20)

project(FakeCAD VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Widgets Gui Core)

add_subdirectory(src)

# 测试集
enable_testing()
add_subdirectory(tests)

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
