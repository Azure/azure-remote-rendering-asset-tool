#pragma once
#include <QVariant>
#include <ViewModel/Parameters/ParameterModel.h>

// Model for a float parameter

class FloatModel : public ParameterModel
{
    Q_OBJECT

public:
    FloatModel(QString name, QObject* object, const std::string_view& parameterName,
               float minimum = s_noMinimum,
               float maximum = s_noMaximum,
               QString format = QString("%.3fm"))
        : ParameterModel(name, object, parameterName)
        , m_minimum(minimum)
        , m_maximum(maximum)
        , m_format(std::move(format))
    {
    }

    float getValue() const
    {
        return ParameterModel::getValue().value<float>();
    }
    void setValue(float value)
    {
        ParameterModel::setValue(QVariant::fromValue<float>(value));
    }

    float getMinimum() const { return m_minimum; }
    float getMaximum() const { return m_maximum; }
    QString getFormat() const { return m_format; }

    static const float s_noMinimum;
    static const float s_noMaximum;

private:
    const float m_minimum;
    const float m_maximum;
    const QString m_format;
};
