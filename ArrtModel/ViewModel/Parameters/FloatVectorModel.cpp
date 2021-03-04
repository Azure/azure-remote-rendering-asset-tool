#include <Model/IncludesAzureRemoteRendering.h>
#include <ViewModel/Parameters/FloatVectorModel.h>

#include <utility>

FloatVectorModel::FloatVectorModel(QString name, QObject* object, const std::string_view& parameterName)
    : ParameterModel(std::move(name), object, parameterName)
{
}

void FloatVectorModel::setLabels(QString label1, QString label2, QString label3, QString label4)
{
    m_labels[0] = std::move(label1);
    m_labels[1] = std::move(label2);
    m_labels[2] = std::move(label3);
    m_labels[3] = std::move(label4);
}

void FloatVectorModel::setFormat(QString format)
{
    m_format = std::move(format);
}

QString FloatVectorModel::getFormat() const
{
    return m_format;
}

QString FloatVectorModel::getLabel(int index) const
{
    assert(index >= 0 || index < 4);
    return m_labels[index];
}


Float2Model::Float2Model(QString name, QObject* object, const std::string_view& parameterName, QString label1, QString label2)
    : FloatVectorModel(std::move(name), object, parameterName)
{
    setLabels(std::move(label1), std::move(label2));
}

float Float2Model::getValue(int index) const
{
    auto v = ParameterModel::getValue().value<RR::Float2>();
    switch (index)
    {
        case 0:
            return v.X;
        case 1:
            return v.Y;
        default:
            throw "Bad index";
    }
}

void Float2Model::setValue(int index, float value)
{
    auto v = ParameterModel::getValue().value<RR::Float2>();
    switch (index)
    {
        case 0:
            v.X = value;
            break;
        case 1:
            v.Y = value;
            break;
        default:
            throw "Bad index";
    }
    ParameterModel::setValue(QVariant::fromValue(v));
}


Float3Model::Float3Model(QString name, QObject* object, const std::string_view& parameterName, QString label1, QString label2, QString label3)
    : FloatVectorModel(std::move(name), object, parameterName)
{
    setLabels(std::move(label1), std::move(label2), std::move(label3));
}

float Float3Model::getValue(int index) const
{
    auto v = ParameterModel::getValue().value<RR::Float3>();
    switch (index)
    {
        case 0:
            return v.X;
        case 1:
            return v.Y;
        case 2:
            return v.Z;
        default:
            throw "Bad index";
    }
}

void Float3Model::setValue(int index, float value)
{
    auto v = ParameterModel::getValue().value<RR::Float3>();
    switch (index)
    {
        case 0:
            v.X = value;
            break;
        case 1:
            v.Y = value;
            break;
        case 2:
            v.Z = value;
            break;
        default:
            throw "Bad index";
    }
    ParameterModel::setValue(QVariant::fromValue(v));
}


Float4Model::Float4Model(QString name, QObject* object, const std::string_view& parameterName, QString label1, QString label2, QString label3, QString label4)
    : FloatVectorModel(std::move(name), object, parameterName)
{
    setLabels(std::move(label1), std::move(label2), std::move(label3), std::move(label4));
}

float Float4Model::getValue(int index) const
{
    auto v = ParameterModel::getValue().value<RR::Float4>();
    switch (index)
    {
        case 0:
            return v.X;
        case 1:
            return v.Y;
        case 2:
            return v.Z;
        case 3:
            return v.W;
        default:
            throw "Bad index";
    }
}

void Float4Model::setValue(int index, float value)
{
    auto v = ParameterModel::getValue().value<RR::Float4>();
    switch (index)
    {
        case 0:
            v.X = value;
            break;
        case 1:
            v.Y = value;
            break;
        case 2:
            v.Z = value;
            break;
        case 3:
            v.W = value;
            break;
        default:
            throw "Bad index";
    }
    ParameterModel::setValue(QVariant::fromValue(v));
}
