#pragma once

#include <QPointer>
#include <View/Parameters/BoundWidget.h>
#include <ViewModel/Parameters/IntegerModel.h>
#include <Widgets/FormatDoubleSpinBox.h>

// Int spin-box bound to a IntegerModel

class BoundIntSpinBox : public FormatDoubleSpinBox, public BoundWidget
{
    Q_OBJECT
    Q_INTERFACES(BoundWidget)

public:
    BoundIntSpinBox(IntegerModel* model, QWidget* parent = nullptr);
    virtual const ParameterModel* getModel() const override;
    void updateFromModel() override;

private:
    QPointer<IntegerModel> m_model;
};
