#pragma once

#include <QPointer>
#include <View/Parameters/BoundWidget.h>
#include <ViewModel/Parameters/ToggleButtonModel.h>
#include <Widgets/FlatButton.h>

// Toggle flatButton bound to a ToggleButtonModel

class BoundToggleButton : public FlatButton, public BoundWidget
{
    Q_OBJECT
    Q_INTERFACES(BoundWidget)

public:
    BoundToggleButton(ToggleButtonModel* model, QWidget* parent = nullptr);
    virtual const ParameterModel* getModel() const override;
    virtual void updateFromModel() override;

private:
    QPointer<ToggleButtonModel> m_model;
};
