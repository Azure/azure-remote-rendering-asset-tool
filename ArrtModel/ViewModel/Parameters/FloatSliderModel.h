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
                     bool expSlider = false,
                     QString format = QString("%.3fm"))
        : FloatModel(name, object, parameterName, minimum, maximum, std::move(format))
        , m_numberOfSteps(numberOfSteps)
        , m_expSlider(expSlider)
    {
    }

    int getNumberOfSteps() const { return m_numberOfSteps; }

    // if true, the slider will be exponential
    bool isExpSlider() const { return m_expSlider; }

private:
    const int m_numberOfSteps;
    const bool m_expSlider;
};
