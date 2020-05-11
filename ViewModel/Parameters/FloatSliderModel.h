#pragma once
#include <ViewModel/Parameters/FloatModel.h>

// Model for a float slider parameter

class FloatSliderModel : public FloatModel
{
    Q_OBJECT

public:
    FloatSliderModel(QString name, QObject* object, const std::string_view& parameterName,
                     float minimum,
                     float maximum,
                     int numberOfSteps,
                     QString format = QString("%.3fm"))
        : FloatModel(name, object, parameterName, minimum, maximum, std::move(format))
        , m_numberOfSteps(numberOfSteps)
    {
    }

    int getNumberOfSteps() const { return m_numberOfSteps; }

private:
    const int m_numberOfSteps;
};
