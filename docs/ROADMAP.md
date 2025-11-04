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
- [x] Circle/Rectangle/LineSegment 模型与视图适配器
- [ ] JSON 保存/加载（基础字段）
- [ ] 选择/移动交互与属性面板
- [ ] 长度/周长/面积计算与渲染标注
- [ ] Triangle/Polygon/Polyline/Ellipse 完整实现
- [ ] 旋转/控制点编辑/快捷键
- [ ] 稳定性与边界处理
- [ ] 文档完善与演示用示例文件

