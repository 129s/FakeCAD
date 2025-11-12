# FakeCAD

一个使用 C++17 + CMake + Qt6（Widgets）构建的简易二维 CAD 项目，用于教学/作业演示：绘制与编辑常见 2D 图形，并计算长度/周长/面积，支持保存/加载。

## 特色
- 基础形状：线段、多段折线、三角形、矩形、N 边形、圆、椭圆。
- 度量：线型长度；区域型周长与面积；就地/面板显示。
- 编辑：选择、拖拽移动、旋转、修改颜色/线型、节点编辑。
- 撤销/重做：添加、删除、移动/旋转、几何编辑。
- 架构：`Shape / LineShape / AreaShape` 抽象层次，模型与视图解耦。
- 持久化：JSON 文件格式，记录图形类型、几何、样式与变换。

## 构建
依赖：
- CMake ≥ 3.20
- C++17 编译器（MSVC/Clang/GCC）
- Qt 6（建议 6.5+，组件：Widgets、Gui、Core）

Windows（MSVC + Ninja 示例）：
```
cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="C:\\Qt\\6.6.2\\msvc2022_64"
cmake --build build -j
```

Windows（MSVC 多配置生成器）：
```
cmake -S . -B build -G "Visual Studio 17 2022" \
  -DCMAKE_PREFIX_PATH="C:\\Qt\\6.6.2\\msvc2022_64"
cmake --build build --config Release -j
```

Linux/macOS：
```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="/path/to/Qt/6.x/gcc_64"
cmake --build build -j
```

运行：
- 可执行文件位于 `build/` 目录（待项目骨架创建后生效）。

## 测试
- 启用 CTest，已提供四类测试：`unit`、`integration`、`ui`、`e2e`（默认需 `-DFAKECAD_BUILD_TESTS=ON` 才会构建）。
- 运行（单配置生成器）：
  - `ctest --test-dir build -VV`
  - 仅运行某类：`ctest --test-dir build -L unit`（支持 `integration`/`ui`/`e2e`）
- 运行（多配置生成器，如 VS）：
  - `ctest --test-dir build -C Debug -VV`
  - `ctest --test-dir build -C Debug -L ui`

## 打包/发布（Windows）
- 目标：生成包含 Qt 运行时依赖的独立包（Release/Debug）。
- 前置：确保系统可找到 `windeployqt.exe`（将 Qt 的 `bin` 目录加入 PATH，或在脚本参数中指定）。

步骤：
- 首次配置（任选其一）：
  - Ninja 示例：`cmake -S . -B build -G "Ninja" -DCMAKE_PREFIX_PATH="C:\\Qt\\6.6.2\\msvc2022_64"`
  - VS 示例：`cmake -S . -B build -G "Visual Studio 17 2022" -DCMAKE_PREFIX_PATH="C:\\Qt\\6.6.2\\msvc2022_64"`
- 一键打包（默认同时生成 Release/Debug ZIP）：

```
powershell -ExecutionPolicy Bypass -File scripts/package.ps1
```

可选参数：
- `-Config Release` 仅打包某一配置。
- `-QtBin C:\\Qt\\6.6.2\\msvc2022_64\\bin` 指定 Qt bin 目录（若未在 PATH）。
- `-Generator "Visual Studio 17 2022"` 首次配置时指定生成器。
- `-BuildDir build` 指定构建目录。
- `-DistDir dist` 指定输出目录。
- `-Mode dynamic|single|both` 控制生成动态包/单 EXE/两者；默认 `dynamic`。
- `-SingleCMakePrefixPath <Qt 静态前缀>`：`-Mode single/both` 时必填。
- `-SizeOptimize`：启用 LTO/节级别剔除（适用于动态与单 EXE）。
- `-UPXPath ... -UPXArgs ...`：对单 EXE 启用 UPX（默认自动检测 `upx.exe`）。

输出：
- `dist/FakeCAD-<version>-win64-Release.zip`
- `dist/FakeCAD-<version>-win64-Debug.zip`

### 单 EXE（静态链接，实验性）
- 说明：需要使用“静态构建”的 Qt（仅 Core/Gui/Widgets 等必要模块），并启用 `FAKECAD_SINGLE_EXE=ON`。遵循 Qt 许可证（LGPL/商业）合规要求。
- 构建与打包：

```
powershell -ExecutionPolicy Bypass -File scripts/package.ps1 -Mode single -Config Release `
  -SingleCMakePrefixPath "C:\Qt-static\6.6.2\msvc2022_64-static" -SizeOptimize
```

- 可选：UPX 压缩

```
powershell -ExecutionPolicy Bypass -File scripts/package.ps1 -Mode single -Config Release `
  -SingleCMakePrefixPath "C:\Qt-static\6.6.2\msvc2022_64-static" -SizeOptimize `
  -UPXPath "C:\tools\upx.exe" -UPXArgs "--best"
```

- 输出：`dist/FakeCAD-<version>-win64-Single-Release.exe`

- 同时生成动态包 + 单 EXE：

```
powershell -ExecutionPolicy Bypass -File scripts/package.ps1 -Mode both -Config Release `
  -Generator "Ninja" -CMakePrefixPath "C:\Qt\6.6.2\msvc2022_64" `
  -SingleCMakePrefixPath "C:\Qt-static\6.6.2\msvc2022_64-static" -SizeOptimize `
  -UPXPath "C:\tools\upx.exe" -UPXArgs "--best"
```

### 二进制瘦身（可选开关）
- 启用 `-DFAKECAD_SIZE_OPTIMIZE=ON`，在 Release/MinsizeRel 中打开 LTO/IPO 与节级别剔除；
- MSVC：`/O2 /GL` + `/OPT:REF /OPT:ICF /INCREMENTAL:NO`；GCC/Clang：`-O2 -ffunction-sections -fdata-sections -Wl,--gc-sections -s`；
- 示例：

```
cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DFAKECAD_SIZE_OPTIMIZE=ON \
  -DCMAKE_PREFIX_PATH="C:\\Qt\\6.6.2\\msvc2022_64"
cmake --build build -j
```

### 清理构建与发布产物
- 预览将删除哪些目录：
```
powershell -ExecutionPolicy Bypass -File scripts/clean.ps1 -DryRun
```
- 一键清理（删除 `build*`、`dist`、`_packages`）：
```
powershell -ExecutionPolicy Bypass -File scripts/clean.ps1 -All
```

## 目录规划（拟定）
- `src/`：核心与 UI 源码
- `include/`：对外头文件（若拆分库）
- `resources/`：图标、样式、翻译等
- `cmake/`：CMake 模块与工具链（可选）
- `tests/`：单元/集成测试（QTest/Catch2 其一）
- `docs/`：文档（需求、设计、路线图等）

## 文档
- 需求说明：`docs/REQUIREMENTS.md`
- 设计说明：`docs/DESIGN.md`
- 路线图：`docs/ROADMAP.md`

## 贡献
- 首次提交将搭建 CMake/Qt6 工程骨架；随后按路线图逐步实现形状与交互。
- 欢迎在 Issue/PR 中讨论改进点（交互、算法、架构等）。
