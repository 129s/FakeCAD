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
#include <QLocale>
#include <cmath>

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
    rotSpin_ = new QDoubleSpinBox(this);
    rotSpin_->setRange(-360.0, 360.0);
    rotSpin_->setSingleStep(1.0);
    lengthEdit_ = new QLineEdit(this);
    perimeterEdit_ = new QLineEdit(this);
    areaEdit_ = new QLineEdit(this);
    for (auto* e : {lengthEdit_, perimeterEdit_, areaEdit_}) {
        e->setReadOnly(true);
    }

    lay->addRow(lblType_);
    lay->addRow(tr("名称"), nameEdit_);
    lay->addRow(tr("颜色"), colorBtn_);
    lay->addRow(tr("线宽"), penWidthSpin_);
    lay->addRow(tr("旋转(°)"), rotSpin_);
    lay->addRow(tr("长度"), lengthEdit_);
    lay->addRow(tr("周长"), perimeterEdit_);
    lay->addRow(tr("面积"), areaEdit_);

    connect(nameEdit_, &QLineEdit::textEdited, this, &PropertyPanel::onNameEdited);
    connect(penWidthSpin_, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &PropertyPanel::onPenWidthChanged);
    connect(colorBtn_, &QPushButton::clicked, this, &PropertyPanel::onColorClicked);
    connect(rotSpin_, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &PropertyPanel::onRotationChanged);
}

void PropertyPanel::setShapeItem(ShapeItem* item) {
    target_ = item;
    refreshFromTarget();
}

void PropertyPanel::clearTarget() {
    target_ = nullptr;
    refreshFromTarget();
}

void PropertyPanel::refresh() { refreshFromTarget(); }

void PropertyPanel::refreshFromTarget() {
    updating_ = true;
    if (!target_) {
        lblType_->setText(tr("类型: -"));
        nameEdit_->setText(QString());
        colorBtn_->setEnabled(false);
        penWidthSpin_->setEnabled(false);
        rotSpin_->setEnabled(false);
        lengthEdit_->setEnabled(false);
        perimeterEdit_->setEnabled(false);
        areaEdit_->setEnabled(false);
        lengthEdit_->setText(QStringLiteral("-"));
        perimeterEdit_->setText(QStringLiteral("-"));
        areaEdit_->setText(QStringLiteral("-"));
        updating_ = false;
        return;
    }
    auto* s = target_->model();
    lblType_->setText(tr("类型: %1").arg(s->typeName()));
    nameEdit_->setText(s->name());
    colorBtn_->setEnabled(true);
    penWidthSpin_->setEnabled(true);
    applyColorToButton(s->pen().color());
    penWidthSpin_->setValue(s->pen().widthF());
    rotSpin_->setEnabled(true);
    rotSpin_->setValue(s->rotationDegrees());
    lengthEdit_->setEnabled(true);
    perimeterEdit_->setEnabled(true);
    areaEdit_->setEnabled(true);

    auto fmt = [](double v) -> QString {
        if (!std::isfinite(v)) return QStringLiteral("-");
        return QLocale().toString(v, 'f', 2);
    };
    lengthEdit_->setText(QStringLiteral("-"));
    perimeterEdit_->setText(QStringLiteral("-"));
    areaEdit_->setText(QStringLiteral("-"));
    if (auto* ls = dynamic_cast<LineShape*>(s)) {
        lengthEdit_->setText(fmt(ls->Length()));
    }
    if (auto* as = dynamic_cast<AreaShape*>(s)) {
        perimeterEdit_->setText(fmt(as->Perimeter()));
        areaEdit_->setText(fmt(as->Area()));
    }
    updating_ = false;
}

void PropertyPanel::applyColorToButton(const QColor& c) {
    QPixmap pm(24, 16);
    pm.fill(c);
    colorBtn_->setIcon(QIcon(pm));
}

void PropertyPanel::onNameEdited(const QString& text) {
    if (updating_ || !target_) return;
    target_->model()->setName(text);
    target_->update();
}

void PropertyPanel::onPenWidthChanged(double w) {
    if (updating_ || !target_) return;
    auto p = target_->model()->pen();
    p.setWidthF(w);
    target_->model()->setPen(p);
    target_->update();
}

void PropertyPanel::onColorClicked() {
    if (!target_) return;
    const QColor cur = target_->model()->pen().color();
    QColor c = QColorDialog::getColor(cur, this, tr("选择颜色"));
    if (!c.isValid()) return;
    auto p = target_->model()->pen();
    p.setColor(c);
    target_->model()->setPen(p);
    applyColorToButton(c);
    target_->update();
}

void PropertyPanel::onRotationChanged(double deg) {
    if (updating_ || !target_) return;
    target_->model()->setRotationDegrees(deg);
    target_->setRotation(deg);
    target_->updateHandles();
}
