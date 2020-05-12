#include <ViewModel/Parameters/ParameterModel.h>

ParameterModel::ParameterModel(QString name, QObject* object, const std::string_view& parameterName)
    : QObject(object)
    , m_name(std::move(name))
    , m_object(object)
    , m_metaProperty(getMetaProperty(object, parameterName))
{
}

QMetaProperty ParameterModel::getMetaProperty(QObject* object, const std::string_view& parameterName)
{
    const int index = object->metaObject()->indexOfProperty(parameterName.data());
    auto metaProperty = object->metaObject()->property(index);
    if (!metaProperty.isValid())
    {
        throw "Property not recognized";
    }
    return metaProperty;
}

QVariant ParameterModel::getValue() const
{
    return m_metaProperty.read(m_object);
}

void ParameterModel::setValue(const QVariant& value)
{
    m_metaProperty.write(m_object, value);
}
