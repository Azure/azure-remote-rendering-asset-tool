#pragma once

#include <QPointer>
#include <QWidget>
#include <View/Parameters/BoundWidget.h>
#include <ViewModel/Parameters/FloatSliderModel.h>

class FloatSliderModel;
class BoundFloatSpinBox;
class QSlider;

// Bound slider for a FloatSliderModel

class BoundFloatSlider : public QWidget, public BoundWidget
{
    Q_OBJECT
    Q_INTERFACES(BoundWidget)

public:
    BoundFloatSlider(FloatSliderModel* model, QWidget* parent = nullptr);
    virtual const ParameterModel* getModel() const override;
    void updateFromModel() override;

private:
    QPointer<FloatSliderModel> m_model;
    BoundFloatSpinBox* m_spinBox;
    QSlider* m_slider;
};
