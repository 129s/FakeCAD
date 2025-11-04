#pragma once

#include <memory>
#include <vector>
#include <QString>
#include <QJsonDocument>
#include "Shape.h"
#include "shapes/LineSegment.h"
#include "shapes/Rectangle.h"
#include "shapes/Circle.h"
#include "shapes/Triangle.h"
#include "shapes/Polygon.h"
#include "shapes/Polyline.h"
#include "shapes/Ellipse.h"

namespace Ser {

QJsonDocument Serialize(const std::vector<Shape*>& shapes);
std::vector<std::unique_ptr<Shape>> Deserialize(const QJsonDocument& doc);

bool SaveToFile(const QString& path, const std::vector<Shape*>& shapes, QString* error = nullptr);
std::vector<std::unique_ptr<Shape>> LoadFromFile(const QString& path, QString* error = nullptr);

// 工具：从单个对象构造 Shape；将 JSON 应用到现有 Shape（类型需匹配）
std::unique_ptr<Shape> FromJsonObject(const QJsonObject& obj);
bool ApplyJsonToShape(Shape* s, const QJsonObject& obj);

}
