#pragma once
#include <QPointer>
#include <QWidget>
#include <View/Parameters/BoundWidget.h>
#include <ViewModel/Parameters/FloatVectorModel.h>

class FormatDoubleSpinBox;

// Widget with 2 to 4 spinboxes, bound to a FloatVectorModel

class BoundFloatVector : public QWidget, public BoundWidget
{
    Q_OBJECT
    Q_INTERFACES(BoundWidget)

public:
    BoundFloatVector(FloatVectorModel* model, QWidget* parent = nullptr);
    virtual const ParameterModel* getModel() const override;
    void updateFromModel() override;

private:
    QPointer<FloatVectorModel> m_model;
    FormatDoubleSpinBox* m_spinBoxes[4];
    const int m_numComponents;
};
