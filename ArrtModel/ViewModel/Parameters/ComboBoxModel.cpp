#include <ViewModel/Parameters/ComboBoxModel.h>

#include <utility>

ComboBoxModel::ComboBoxModel(QString name, QObject* object, const std::string_view& parameterName)
    : ParameterModel(std::move(name), object, parameterName)
    , m_enumerator(m_metaProperty.enumerator())
{
}

int ComboBoxModel::getCount() const
{
    return m_enumerator.keyCount();
}

QString ComboBoxModel::getName(int index) const
{
    return QString::fromUtf8(m_enumerator.key(index));
}

int ComboBoxModel::getValue(int index) const
{
    return m_enumerator.value(index);
}

int ComboBoxModel::getIndex() const
{
    const int value = ParameterModel::getValue().value<int>();
    //check if it makes sense to store a map
    int index = getCount() - 1;
    for (; index >= 0; --index)
    {
        if (getValue(index) == value)
        {
            break;
        }
    }
    return index;
}

void ComboBoxModel::setIndex(int index)
{
    ParameterModel::setValue(QVariant::fromValue<int>(getValue(index)));
}
