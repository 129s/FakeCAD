#include "PropertyPanel.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QColorDialog>
#include <QIcon>
#include <QPixmap>

#include "ShapeItem.h"
#include "../core/Shape.h"

PropertyPanel::PropertyPanel(QWidget* parent)
    : QWidget(parent) {
    rebuildUI();
}

void PropertyPanel::rebuildUI() {
    auto* lay = new QFormLayout(this);
    lblType_ = new QLabel(tr("类型: -"), this);
    nameEdit_ = new QLineEdit(this);
    colorBtn_ = new QPushButton(tr("选择颜色"), this);
    penWidthSpin_ = new QDoubleSpinBox(this);
    penWidthSpin_->setRange(0.1, 50.0);
    penWidthSpin_->setSingleStep(0.5);

    lay->addRow(lblType_);
    lay->addRow(tr("名称"), nameEdit_);
    lay->addRow(tr("颜色"), colorBtn_);
    lay->addRow(tr("线宽"), penWidthSpin_);

    connect(nameEdit_, &QLineEdit::textEdited, this, &PropertyPanel::onNameEdited);
    connect(penWidthSpin_, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &PropertyPanel::onPenWidthChanged);
    connect(colorBtn_, &QPushButton::clicked, this, &PropertyPanel::onColorClicked);
}

void PropertyPanel::setShapeItem(ShapeItem* item) {
    target_ = item;
    refreshFromTarget();
}

void PropertyPanel::clearTarget() {
    target_ = nullptr;
    refreshFromTarget();
}

void PropertyPanel::refreshFromTarget() {
    updating_ = true;
    if (!target_) {
        lblType_->setText(tr("类型: -"));
        nameEdit_->setText(QString());
        colorBtn_->setEnabled(false);
        penWidthSpin_->setEnabled(false);
        updating_ = false;
        return;
    }
    auto* s = target_->shape();
    lblType_->setText(tr("类型: %1").arg(s->typeName()));
    nameEdit_->setText(s->name());
    colorBtn_->setEnabled(true);
    penWidthSpin_->setEnabled(true);
    applyColorToButton(s->pen().color());
    penWidthSpin_->setValue(s->pen().widthF());
    updating_ = false;
}

void PropertyPanel::applyColorToButton(const QColor& c) {
    QPixmap pm(24, 16);
    pm.fill(c);
    colorBtn_->setIcon(QIcon(pm));
}

void PropertyPanel::onNameEdited(const QString& text) {
    if (updating_ || !target_) return;
    target_->shape()->setName(text);
    target_->update();
}

void PropertyPanel::onPenWidthChanged(double w) {
    if (updating_ || !target_) return;
    auto p = target_->shape()->pen();
    p.setWidthF(w);
    target_->shape()->setPen(p);
    target_->update();
}

void PropertyPanel::onColorClicked() {
    if (!target_) return;
    const QColor cur = target_->shape()->pen().color();
    QColor c = QColorDialog::getColor(cur, this, tr("选择颜色"));
    if (!c.isValid()) return;
    auto p = target_->shape()->pen();
    p.setColor(c);
    target_->shape()->setPen(p);
    applyColorToButton(c);
    target_->update();
}

