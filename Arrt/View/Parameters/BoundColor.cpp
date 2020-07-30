#include <View/Parameters/BoundColor.h>


BoundColor::BoundColor(ColorModel* model, QWidget* parent)
    : ColorPicker(parent)
    , m_model(model)
{
    setNotifyWhileSelecting(true);
    setUseAlpha(true);

    BoundColor::updateFromModel();

    QObject::connect(this, &ColorPicker::colorChanged, this, [this]() {
        m_model->setValue(getColor());
    });
}


const ParameterModel* BoundColor::getModel() const
{
    return m_model;
}

void BoundColor::updateFromModel()
{
    setColor(m_model->getValue());
}
