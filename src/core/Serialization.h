#pragma once

#include <memory>
#include <vector>
#include <QString>
#include <QJsonDocument>
#include "Shape.h"

namespace Ser {

QJsonDocument Serialize(const std::vector<Shape*>& shapes);
std::vector<std::unique_ptr<Shape>> Deserialize(const QJsonDocument& doc);

bool SaveToFile(const QString& path, const std::vector<Shape*>& shapes, QString* error = nullptr);
std::vector<std::unique_ptr<Shape>> LoadFromFile(const QString& path, QString* error = nullptr);

}

