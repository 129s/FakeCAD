# FakeCAD 架构与设计

## 技术选型
- 语言/构建：C++17、CMake
- GUI 框架：Qt6 Widgets（QMainWindow + QGraphicsView/Scene）
- 渲染/交互：QGraphicsScene/QGraphicsView（缩放、平移、选择、项交互）
- 序列化：Qt JSON（QJsonObject/QJsonArray/QJsonDocument）

## 总体架构
- 核心模型（Core）：图形数据模型与几何计算（与 Qt GUI 解耦）。
- 视图层（UI）：基于 QGraphicsItem 的可视元素与交互控制。
- 适配层（Adapter）：Model ↔ QGraphicsItem 的绑定与事件转发。
- 持久化（Persistence）：模型的 JSON 读写（版本化）。

## 类设计（模型层）
- `class Shape`
  - 数据：`std::string name`，`QColor color`，`QPen pen`，`QTransform transform`
  - 接口：
    - 变换：`Move(dx,dy)`、`MoveTo(x,y)`、`Rotate(angle[, cx, cy])`
    - 度量：`virtual double Length() const`（区域默认返回 Perimeter）
    - 辅助：`virtual QRectF BoundingBox() const = 0`
    - 序列化：`virtual QJsonObject ToJson() const` / `static std::unique_ptr<Shape> FromJson(const QJsonObject&)`

- `class LineShape : public Shape`
  - 接口：`virtual int VertexCount() const = 0`，`virtual QVector<QPointF> Vertices() const = 0`
  - 度量：`double Length() const override`（基于顶点累加）

- `class AreaShape : public Shape`
  - 接口：`virtual double Area() const = 0`
  - 度量：`double Perimeter() const`（= `Length()` 或单独实现）

- 具体图形（均含静态计数器 `inline static std::atomic<int> kCount`）：
  - `LineSegment`：两个端点 `p1, p2`
  - `Polyline`：`QVector<QPointF> points`
  - `Triangle`：三个顶点
  - `Rectangle`：原点+宽高或四顶点；支持旋转
  - `Polygon`：`QVector<QPointF>`（简单多边形）
  - `Circle`：中心、半径
  - `Ellipse`：中心、半轴（rx, ry）

说明：
- 变换采用 `QTransform` 统一管理平移/旋转/缩放，几何计算时按需要应用逆变换或在局部坐标系计算。
- 面积/周长：
  - 多边形：Shoelace 公式 + 邻边距离求和
  - 椭圆周长：Ramanujan 近似（或数值逼近）

## 视图与交互
- `QGraphicsScene` 管理 `QGraphicsItem` 项；每个模型 `Shape` 对应一个 `ShapeItem`（适配器）。
- `ShapeItem` 负责：
  - 呈现：`paint()` 使用模型颜色/线型；
  - 选取与拖拽：启用 `ItemIsSelectable`、`ItemIsMovable`；
  - 控制点：在编辑模式下显示/拖动顶点或几何句柄；
  - 同步：交互修改 → 更新模型；模型变更 → 触发 `update()`。
- 主窗体：菜单/工具栏（绘制模式切换、打开/保存、撤销重做）、属性面板（选中项属性编辑）、状态栏（提示）

## 序列化设计
通用结构：
```json
{
  "version": 1,
  "scene": {
    "width": 1920,
    "height": 1080
  },
  "shapes": [
    {
      "type": "Circle",
      "name": "c1",
      "style": {"color": "#ff0000", "pen": {"width": 2, "style": "DashLine"}},
      "geom": {"cx": 100, "cy": 200, "r": 80},
      "transform": {"tx": 0, "ty": 0, "rot": 0}
    }
  ]
}
```
说明：
- `type` 用于反序列化分派；`geom` 存放关键几何参数；`transform` 存放平移/旋转（可扩展缩放）。
- 颜色/线型基于可读字符串或枚举名；读写时做映射。

## 误差与健壮性
- 退化情形：零长度边、共线三点、多边形自交（加载时拒绝或修正）。
- 数值稳定：epsilon 比较；最小尺寸阈值；避免除零与 NaN。

## 测试策略
- 几何单元测试：长度/周长/面积、包围盒、碰撞/命中测试。
- 序列化用例：覆盖所有图形的读写往返（round-trip）。
- 交互回归：基础绘制、选择/移动、旋转的手动脚本或自动化（可选）。

