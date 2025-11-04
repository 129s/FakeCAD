# FakeCAD

一个使用 C++17 + CMake + Qt6（Widgets）构建的简易二维 CAD 项目，用于教学/作业演示：绘制与编辑常见 2D 图形，并计算长度/周长/面积，支持保存/加载。

## 特色
- 基础形状：线段、多段折线、三角形、矩形、N 边形、圆、椭圆。
- 度量：线型长度；区域型周长与面积；就地/面板显示。
- 编辑：选择、拖拽移动、旋转、修改颜色/线型、节点编辑。
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

