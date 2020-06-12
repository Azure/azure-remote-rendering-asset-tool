#include <View/Parameters/BoundCheckBox.h>

BoundCheckBox::BoundCheckBox(CheckBoxModel* model, QWidget* parent)
    : QCheckBox(parent)
    , m_model(model)
{
    BoundCheckBox::updateFromModel();

    QObject::connect(this, &QCheckBox::stateChanged, this, [this](int state) {
        m_model->setValue(state == Qt::CheckState::Checked);
    });
}


const ParameterModel* BoundCheckBox::getModel() const
{
    return m_model;
}

void BoundCheckBox::updateFromModel()
{
    setChecked(m_model->getValue());
}
