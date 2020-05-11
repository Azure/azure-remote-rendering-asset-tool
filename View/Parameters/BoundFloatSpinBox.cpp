#include <View/Parameters/BoundFloatSpinBox.h>

BoundFloatSpinBox::BoundFloatSpinBox(FloatModel* model, QWidget* parent)
    : FormatDoubleSpinBox(parent, model->getFormat(), NumberFormatter::DOUBLE_FORMAT)
    , m_model(model)
{
    setMinimum(m_model->getMinimum());
    setMaximum(m_model->getMaximum());
    setStep(1.0f / 100);

    BoundFloatSpinBox::updateFromModel();

    QObject::connect(this, &FormatDoubleSpinBox::edited, this, [this]() {
        m_model->setValue(value());
    });
}


const ParameterModel* BoundFloatSpinBox::getModel() const
{
    return m_model;
}

void BoundFloatSpinBox::updateFromModel()
{
    setValue(m_model->getValue());
}
