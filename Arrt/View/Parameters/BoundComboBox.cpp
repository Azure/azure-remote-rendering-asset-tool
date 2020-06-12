#include <View/Parameters/BoundComboBox.h>

BoundCombobox::BoundCombobox(ComboBoxModel* model, QWidget* parent)
    : CustomComboBox(parent)
    , m_model(model)
{
    for (int i = 0; i < m_model->getCount(); ++i)
    {
        addItem(m_model->getName(i), m_model->getValue(i));
    }
    BoundCombobox::updateFromModel();

    QObject::connect(this, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this](int index) {
        m_model->setIndex(index);
    });
}


const ParameterModel* BoundCombobox::getModel() const
{
    return m_model;
}

void BoundCombobox::updateFromModel()
{
    setCurrentIndex(m_model->getIndex());
}
