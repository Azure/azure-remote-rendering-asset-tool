#include <ViewModel/Parameters/MultiComboBoxModel.h>

#include <utility>

MultiComboBoxModel::MultiComboBoxModel(QString name, QObject* object, const std::string_view& parameterName)
    : ParameterModel(std::move(name), object, parameterName)
    , m_enumerator(m_metaProperty.enumerator())
{
}

int MultiComboBoxModel::getCount() const
{
    return m_enumerator.keyCount();
}

QString MultiComboBoxModel::getEnumName(int index) const
{
    return QString::fromUtf8(m_enumerator.key(index));
}

int MultiComboBoxModel::getEnumValue(int index) const
{
    return m_enumerator.value(index);
}

int MultiComboBoxModel::getMask() const
{
    return ParameterModel::getValue().value<int>();
}

void MultiComboBoxModel::setMask(int mask)
{
    ParameterModel::setValue(QVariant::fromValue<int>(mask));
}
