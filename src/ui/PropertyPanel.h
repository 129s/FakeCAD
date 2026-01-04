#pragma once

#include <QWidget>

class QLineEdit;
class QPushButton;
class QDoubleSpinBox;
class QLabel;
class ShapeItem;

class PropertyPanel : public QWidget {
    Q_OBJECT
 public:
    explicit PropertyPanel(QWidget* parent = nullptr);

    void setShapeItem(ShapeItem* item);
    void clearTarget();

public slots:
    void refresh();

 private slots:
    void onNameEdited(const QString& text);
    void onPenWidthChanged(double w);
    void onColorClicked();
    void onRotationChanged(double deg);

private:
    void rebuildUI();
    void refreshFromTarget();
    void applyColorToButton(const QColor& c);

    ShapeItem* target_ { nullptr };
    bool updating_ { false };

    QLabel* lblType_ {};
    QLineEdit* nameEdit_ {};
    QPushButton* colorBtn_ {};
    QDoubleSpinBox* penWidthSpin_ {};
    QDoubleSpinBox* rotSpin_ {};
    QLineEdit* lengthEdit_ {};
    QLineEdit* perimeterEdit_ {};
    QLineEdit* areaEdit_ {};
};
