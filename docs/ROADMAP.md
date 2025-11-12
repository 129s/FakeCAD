# FakeCAD 路线图与任务清单

## 里程碑
1. 工程骨架
   - 初始化 CMake 与 Qt6 Widgets 应用
   - 引入基础目录结构与工具脚本

2. 模型与持久化基础
   - 建立 `Shape / LineShape / AreaShape` 抽象
   - 实现 Circle、Rectangle、LineSegment 三种代表性图形
   - JSON 序列化/反序列化框架与版本字段

3. 视图与交互
   - QGraphicsView/Scene 画布；平移/缩放
   - 选择与拖拽移动；属性面板显示与编辑
   - 绘制流程（点击/拖拽创建）：线段、矩形、圆

4. 扩展图形
   - Triangle、Polygon、Polyline、Ellipse
   - 节点/控制点编辑；旋转控制

5. 计算与显示
   - 长度/周长/面积的准确计算与文本标注
   - 统计面板：各图形实例数与总览

6. 稳定与发布
   - 边界与异常处理；用户体验打磨
   - 打包与发布说明（Windows 优先）

## 任务清单（建议顺序）
- [x] 初始化 CMake 项目（Qt6 Widgets 模板）
- [x] 搭建基础 UI（主窗体、菜单、工具栏、状态栏）
- [x] 引入 QGraphicsView/Scene 画布
- [x] 定义抽象类与通用属性/方法
- [x] 实现基础三形模型（线/矩形/圆）
- [x] Circle/Rectangle/LineSegment 模型与视图适配器
- [x] JSON 保存/加载（基础字段）
- [x] 绘制流程（点击/拖拽创建）：线段、矩形、圆
- [x] 选择/移动交互与属性面板
- [x] 长度/周长/面积计算与渲染标注
- [x] Triangle/Polygon/Polyline/Ellipse 完整实现
- [x] 旋转/控制点编辑
- [x] 快捷键（工具切换/视图缩放/网格切换/删除/Esc）
- [x] 稳定性与边界处理（退化图形过滤、属性面板指针清理、网格绘制cosmetic、缩放范围限制）
- [x] 撤销/重做（添加/删除/变换/几何编辑）
- [ ] 文档完善与演示用示例文件

### 发布优化：静态链接单 EXE 与瘦身

- [ ] 静态链接单文件（Windows 优先）：提供“真·单 EXE”方案（Qt 静态）
- [ ] 构建瘦身：LTO/GC sections/strip（GCC/Clang）或 /OPT:REF,/ICF（MSVC）
- [ ] 可执行压缩（可选）：UPX 压缩与启动/误报评估
- [ ] 自解压单文件替代方案（无静态 Qt）：Inno Setup/7zSFX 一体化分发

#### 方案说明与任务拆分

1) 静态链接单 EXE（Qt 静态库）
   - 目标：在无 Qt 运行时的机器上也能直接运行，仅分发一个 `fakecad.exe`。
   - 注意：Qt 静态链接涉及许可证约束（LGPL）。若使用 LGPL 版本的 Qt 静态库，需要向用户提供可重新链接的目标文件或其他合规手段；或使用商业许可。默认仍保留基于 windeployqt 的动态分发方案。
   - 任务：
     - 新增 CMake 选项：`FAKECAD_SINGLE_EXE`（默认 OFF）。
       - MSVC：`CMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded`（/MT 静态运行库）。
       - MinGW：链接选项示例 `-static -static-libstdc++ -static-libgcc -s -ffunction-sections -fdata-sections -Wl,--gc-sections`。
     - 编译 Qt 静态库（仅需要的模块：Core/Gui/Widgets）：
       - Windows（示例，按本地版本调整）：
         - 进入 Qt 源码根目录，执行 `configure.bat`：
           - MSVC：`configure.bat -static -static-runtime -release -opensource -confirm-license -nomake tests -nomake examples -skip qtwebengine`
           - MinGW：`configure.bat -static -release -opensource -confirm-license -nomake tests -nomake examples -skip qtwebengine`
         - 之后使用 CMake/Ninja 构建并安装，记录安装前缀，作为 `CMAKE_PREFIX_PATH`。
     - 静态平台插件导入（Windows）：在启用 `FAKECAD_SINGLE_EXE` 时为 `main.cpp` 增加：
       - `#include <QtPlugin>` 与 `Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)`（仅静态构建时启用）。
     - 打包脚本：统一在 `scripts/package.ps1` 中提供 dynamic/single/both 三模式，单 EXE 输出仅复制一个 `fakecad.exe` 与必要的 `LICENSE/NOTICE`（若 LGPL 静态需附带目标文件或 relink 说明）。
     - CI（可选）：增加 MSVC/MinGW + Qt 静态矩阵，生成构建体积与启动时间对比报告。

2) 单文件替代方案（无需 Qt 静态）
   - 若不采用 Qt 静态，可通过自解压打包器获得“单文件安装/运行”体验：
     - Inno Setup 生成单 EXE 安装包，或 7zSFX/自解压将 `windeployqt` 的输出打包为一个运行器，启动时解压至临时目录后运行。
   - 优势：规避 Qt 静态的许可证复杂度与构建成本；劣势：严格意义上并非“真·单可执行”。

3) 瘦身计划（二进制体积与依赖）
   - 依赖裁剪：仅链接 Qt Core/Gui/Widgets；不启用未使用模块（Network/WebEngine/QML 等）。
   - 编译优化：
     - 通用：开启 LTO/IPO：`CMAKE_INTERPROCEDURAL_OPTIMIZATION=ON`（支持的工具链）。
     - GCC/Clang：`-O2 -DNDEBUG -ffunction-sections -fdata-sections`，链接 `-Wl,--gc-sections -s`。
     - MSVC：编译 `/O2 /GL`，链接 `/OPT:REF /OPT:ICF /INCREMENTAL:NO`，Release 版本不生成 PDB（或单独归档）。
   - 资源优化：
     - Qt rcc 压缩：在 `qt_add_resources` 中使用高压缩（如 `--compress 9 --compress-algo zstd`，按 Qt 版本支持情况调整）。
     - 清理未用图标/翻译/示例数据；使用矢量图标优先。
   - 运行库取舍：
     - 单 EXE 场景一般使用静态运行库（体积更大，分发更简单）。
     - 若追求极小体积且允许携带 DLL，可保留动态 CRT（非单文件）。
   - 可执行压缩（可选）：
     - UPX 压缩单 EXE，观察启动时间与杀软误报；提供开关（CI 与本地均可一键启用/禁用）。

4) 验收标准
   - 在干净 Windows 环境（未安装 Qt）上，单 EXE 可直接运行（静态方案）。
   - 输出体积、启动时间与依赖情况有对比表；默认分发仍保持稳定的动态打包方案。
   - 针对压缩/优化后的二进制，功能与稳定性不回退（关键路径测试通过）。

### 体验优化
- [x] 滚轮缩放（视图锚定鼠标）
- [x] 空格+左键平移
- [x] 网格显示与吸附



