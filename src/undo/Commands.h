#pragma once

#include <QUndoCommand>
#include <QPointer>
#include <QJsonObject>
#include <vector>

class DrawingScene;
class ShapeItem;
class Shape;

namespace UndoCmd {

class AddShapeCommand : public QUndoCommand {
public:
    AddShapeCommand(DrawingScene* scene, const QJsonObject& shapeJson, QUndoCommand* parent = nullptr);
    void undo() override;
    void redo() override;
private:
    QPointer<DrawingScene> scene_;
    QJsonObject json_;
    QPointer<ShapeItem> item_{};
};

class DeleteShapesCommand : public QUndoCommand {
public:
    DeleteShapesCommand(DrawingScene* scene, const std::vector<QJsonObject>& shapes, QUndoCommand* parent = nullptr);
    void undo() override;
    void redo() override;
private:
    QPointer<DrawingScene> scene_;
    std::vector<QJsonObject> jsons_;
    std::vector<QPointer<ShapeItem>> items_;
};

class TransformShapeCommand : public QUndoCommand {
public:
    TransformShapeCommand(ShapeItem* item, const QPointF& oldPos, double oldRot, const QPointF& newPos, double newRot, QUndoCommand* parent = nullptr);
    void undo() override;
    void redo() override;
private:
    QPointer<ShapeItem> item_;
    QPointF oldPos_{}; double oldRot_{};
    QPointF newPos_{}; double newRot_{};
    void apply(const QPointF& pos, double rot);
};

class EditShapeJsonCommand : public QUndoCommand {
public:
    EditShapeJsonCommand(ShapeItem* item, const QJsonObject& oldJ, const QJsonObject& newJ, QUndoCommand* parent = nullptr);
    void undo() override;
    void redo() override;
private:
    QPointer<ShapeItem> item_;
    QJsonObject old_, neo_;
    void apply(const QJsonObject& j);
};

}

