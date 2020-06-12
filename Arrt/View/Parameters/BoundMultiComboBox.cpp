#include <View/Parameters/BoundMultiComboBox.h>


BoundMultiComboBox::BoundMultiComboBox(MultiComboBoxModel* model, QWidget* parent)
    : CheckComboBox(parent)
    , m_model(model)
{
    for (int i = 0; i < m_model->getCount(); ++i)
    {
        addItem(m_model->getEnumName(i), m_model->getEnumValue(i));
    }
    insertSeparator(count());
    addSelectAllItem();

    BoundMultiComboBox::updateFromModel();

    QObject::connect(this, &CheckComboBox::dataChanged, this, [this]() {
        int value = 0;

        for (int i = 0; i < m_model->getCount(); i++)
        {
            if (!isValidIndex(i))
            {
                continue;
            }
            const int valueBits = m_model->getEnumValue(i);

            Qt::CheckState state = getCheckState(i);

            if (state == Qt::Checked)
            {
                value |= valueBits;
            }
        }
        m_model->setMask(value);
    });
}

const ParameterModel* BoundMultiComboBox::getModel() const
{
    return m_model;
}

void BoundMultiComboBox::updateFromModel()
{
    int mask = m_model->getMask();

    for (int i = 0; i < m_model->getCount(); ++i)
    {
        setItemData(i, (m_model->getEnumValue(i) & mask) != 0 ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);
    }
}
