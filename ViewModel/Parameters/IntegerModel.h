#pragma once
#include <QVariant>
#include <ViewModel/Parameters/ParameterModel.h>

// model for a BoundIntSpinBox, over an integer property

class IntegerModel : public ParameterModel
{
    Q_OBJECT

public:
    IntegerModel(QString name, QObject* object, const std::string_view& parameterName,
                 int minimum = s_noMinimum,
                 int maximum = s_noMaximum,
                 int step = 1)
        : ParameterModel(name, object, parameterName)
        , m_minimum(minimum)
        , m_maximum(maximum)
        , m_step(step)
    {
        assert(minimum < maximum);
    }

    int getValue() const
    {
        return ParameterModel::getValue().value<int>();
    }
    void setValue(float value)
    {
        ParameterModel::setValue(QVariant::fromValue<int>(value));
    }

    int getMinimum() const { return m_minimum; }
    int getMaximum() const { return m_maximum; }
    int getStep() const { return m_step; }

    static const int s_noMinimum = std::numeric_limits<int>().min();
    static const int s_noMaximum = std::numeric_limits<int>().max();

private:
    const int m_minimum;
    const int m_maximum;
    const int m_step;
};
