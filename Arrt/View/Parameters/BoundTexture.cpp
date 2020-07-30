#include <View/Parameters/BoundTexture.h>

BoundTexture::BoundTexture(TextureModel* model, QWidget* parent)
    : QLineEdit(parent)
    , m_model(model)
{
    setReadOnly(true);
    BoundTexture::updateFromModel();
}

const ParameterModel* BoundTexture::getModel() const
{
    return m_model;
}

void BoundTexture::updateFromModel()
{
    setText(m_model->getValue());
}
