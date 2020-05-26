#include <View/Parameters/BoundText.h>

BoundText::BoundText(TextModel* model, QWidget* parent)
    : QLineEdit(parent)
    , m_model(model)
{
    if (m_model->m_isPassword)
    {
        setEchoMode(QLineEdit::Password);
    }

    BoundText::updateFromModel();

    if (m_model->m_updateOnEditingFinished)
    {
        QObject::connect(this, &QLineEdit::editingFinished, this, [this]() {
            m_model->setValue(this->text());
        });
    }
    else
    {
        QObject::connect(this, &QLineEdit::textChanged, this, [this](const QString& text) {
            m_model->setValue(text);
        });
    }
}


const ParameterModel* BoundText::getModel() const
{
    return m_model;
}

void BoundText::updateFromModel()
{
    setText(m_model->getValue());
}
