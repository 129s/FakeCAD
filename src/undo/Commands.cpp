#include "Commands.h"

#include <QGraphicsScene>

#include "../ui/DrawingScene.h"
#include "../ui/ShapeItem.h"
#include "../core/Serialization.h"

using namespace UndoCmd;

AddShapeCommand::AddShapeCommand(DrawingScene* scene, const QJsonObject& shapeJson, QUndoCommand* parent)
    : QUndoCommand(QObject::tr("添加图形"), parent), scene_(scene), json_(shapeJson) {}

void AddShapeCommand::redo() {
    if (!scene_) return;
    auto shape = Ser::FromJsonObject(json_);
    if (!shape) return;
    auto* item = new ShapeItem(std::move(shape));
    scene_->addItem(item);
    item_ = item;
}

void AddShapeCommand::undo() {
    if (item_) {
        scene_->removeItem(item_);
        delete item_;
        item_ = nullptr;
    }
}

DeleteShapesCommand::DeleteShapesCommand(DrawingScene* scene, const std::vector<QJsonObject>& shapes, QUndoCommand* parent)
    : QUndoCommand(QObject::tr("删除图形"), parent), scene_(scene), jsons_(shapes) {}

void DeleteShapesCommand::redo() {
    if (!scene_) return;
    if (!items_.empty()) {
        // 已经创建过，直接删
        for (auto& it : items_) {
            if (it) { scene_->removeItem(it); delete it; }
        }
        items_.clear();
        return;
    }
    // 初次执行：根据当前选中删除
    auto sel = scene_->selectedItems();
    for (auto* it : sel) {
        if (auto* si = dynamic_cast<ShapeItem*>(it)) {
            items_.push_back(si);
        }
    }
    for (auto& it : items_) { scene_->removeItem(it); }
    // 延后统一删除以避免迭代失效
    for (auto& it : items_) { delete it; }
    items_.clear();
}

void DeleteShapesCommand::undo() {
    if (!scene_) return;
    for (const auto& j : jsons_) {
        auto s = Ser::FromJsonObject(j);
        if (!s) continue;
        auto* item = new ShapeItem(std::move(s));
        scene_->addItem(item);
        items_.push_back(item);
    }
}

TransformShapeCommand::TransformShapeCommand(ShapeItem* item, const QPointF& oldPos, double oldRot, const QPointF& newPos, double newRot, QUndoCommand* parent)
    : QUndoCommand(QObject::tr("变换"), parent), item_(item), oldPos_(oldPos), oldRot_(oldRot), newPos_(newPos), newRot_(newRot) {}

void TransformShapeCommand::apply(const QPointF& pos, double rot) {
    if (!item_) return;
    item_->setPos(pos);
    item_->setRotation(rot);
    if (item_->model()) {
        item_->model()->MoveTo(pos.x(), pos.y());
        item_->model()->setRotationDegrees(rot);
    }
    item_->updateHandles();
}

void TransformShapeCommand::redo() { apply(newPos_, newRot_); }
void TransformShapeCommand::undo() { apply(oldPos_, oldRot_); }

EditShapeJsonCommand::EditShapeJsonCommand(ShapeItem* item, const QJsonObject& oldJ, const QJsonObject& newJ, QUndoCommand* parent)
    : QUndoCommand(QObject::tr("编辑几何"), parent), item_(item), old_(oldJ), neo_(newJ) {}

void EditShapeJsonCommand::apply(const QJsonObject& j) {
    if (!item_ || !item_->model()) return;
    Ser::ApplyJsonToShape(item_->model(), j);
    item_->geometryChanged();
    item_->updateHandles();
    item_->update();
}

void EditShapeJsonCommand::redo() { apply(neo_); }
void EditShapeJsonCommand::undo() { apply(old_); }
