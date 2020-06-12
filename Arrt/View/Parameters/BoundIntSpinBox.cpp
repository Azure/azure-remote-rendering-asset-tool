#include <View/Parameters/BoundIntSpinBox.h>

BoundIntSpinBox::BoundIntSpinBox(IntegerModel* model, QWidget* parent)
    : FormatDoubleSpinBox(parent, {}, NumberFormatter::INTEGER_FORMAT)
    , m_model(model)
{
    setMinimum(m_model->getMinimum());
    setMaximum(m_model->getMaximum());
    setStep(m_model->getStep());
    BoundIntSpinBox::updateFromModel();

    QObject::connect(this, &FormatDoubleSpinBox::edited, this, [this]() {
        m_model->setValue((int)std::round(value()));
    });
}


const ParameterModel* BoundIntSpinBox::getModel() const
{
    return m_model;
}

void BoundIntSpinBox::updateFromModel()
{
    setValue(m_model->getValue());
}
