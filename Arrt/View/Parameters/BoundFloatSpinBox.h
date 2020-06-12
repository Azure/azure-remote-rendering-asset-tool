#pragma once

#include <QPointer>
#include <View/Parameters/BoundWidget.h>
#include <ViewModel/Parameters/FloatModel.h>
#include <Widgets/FormatDoubleSpinBox.h>

// Bound spinbox for a FloatModel

class BoundFloatSpinBox : public FormatDoubleSpinBox, public BoundWidget
{
    Q_OBJECT
    Q_INTERFACES(BoundWidget)

public:
    BoundFloatSpinBox(FloatModel* model, QWidget* parent = nullptr);
    virtual const ParameterModel* getModel() const override;
    void updateFromModel() override;

private:
    QPointer<FloatModel> m_model;
};
