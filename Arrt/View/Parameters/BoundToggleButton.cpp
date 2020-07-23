#include <View/Parameters/BoundToggleButton.h>

BoundToggleButton::BoundToggleButton(ToggleButtonModel* model, QWidget* parent)
    : FlatButton(model->getName(), parent)
    , m_model(model)
{
    setCheckable(true);
    BoundToggleButton::updateFromModel();

    QObject::connect(this, &FlatButton::toggled, this, [this]() {
        m_model->setValue(isChecked());
    });
}


const ParameterModel* BoundToggleButton::getModel() const
{
    return m_model;
}

void BoundToggleButton::updateFromModel()
{
    setChecked(m_model->getValue());
}
