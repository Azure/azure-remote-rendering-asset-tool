#pragma once
#include <QComboBox>
#include <View/Parameters/BoundWidget.h>
#include <ViewModel/Parameters/ColorModel.h>
#include <Widgets/ColorPicker.h>

// ColorPicker bound to a ColorModel

class BoundColor : public ColorPicker, public BoundWidget
{
    Q_OBJECT
    Q_INTERFACES(BoundWidget)

public:
    BoundColor(ColorModel* model, QWidget* parent = nullptr);
    virtual const ParameterModel* getModel() const override;
    void updateFromModel() override;

private:
    QPointer<ColorModel> m_model;
};
