#include <ViewModel/Parameters/ComboBoxModel.h>

#include <utility>


ComboBoxModel::ComboBoxModel(QString name, QObject* object, const std::string_view& parameterName)
    : ParameterModel(std::move(name), object, parameterName)
{
}

int ComboBoxModel::getIndex() const
{
    const QVariant value = ParameterModel::getValue();
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
    ParameterModel::setValue(getValue(index));
}

ComboBoxModelFromEnum::ComboBoxModelFromEnum(QString name, QObject* object, const std::string_view& parameterName)
    : ComboBoxModel(std::move(name), object, parameterName)
    , m_enumerator(m_metaProperty.enumerator())
{
}

int ComboBoxModelFromEnum::getCount() const
{
    return m_enumerator.keyCount();
}

QString ComboBoxModelFromEnum::getName(int index) const
{
    return QString::fromUtf8(m_enumerator.key(index));
}

QVariant ComboBoxModelFromEnum::getValue(int index) const
{
    return m_enumerator.value(index);
}


void ComboBoxModelFromMap::addEntry(QString name, QVariant value)
{
    m_entries.push_back({std::move(name), std::move(value)});
}

int ComboBoxModelFromMap::getCount() const
{
    return static_cast<int>(m_entries.size());
}

QString ComboBoxModelFromMap::getName(int index) const
{
    return m_entries[index].m_label;
}

QVariant ComboBoxModelFromMap::getValue(int index) const
{
    return m_entries[index].m_value;
}
