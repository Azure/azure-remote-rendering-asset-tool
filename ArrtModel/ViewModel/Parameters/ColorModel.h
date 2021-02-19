#pragma once
#include <Model/IncludesAzureRemoteRendering.h>
#include <QColor>
#include <QVariant>
#include <ViewModel/Parameters/ParameterModel.h>

// model for a color parameter, converting RR:Color4 to QColor

class ColorModel : public ParameterModel
{
    Q_OBJECT

public:
    ColorModel(QString name, QObject* object, const std::string_view& parameterName)
        : ParameterModel(name, object, parameterName)
    {
    }

    QColor getValue() const
    {
        auto c = ParameterModel::getValue().value<RR::Color4>();
        QColor qColor;
        qColor.setRedF(c.R);
        qColor.setGreenF(c.G);
        qColor.setBlueF(c.B);
        qColor.setAlphaF(c.A);
        return qColor;
    }
    void setValue(const QColor& qValue)
    {
        RR::Color4 value = {(float)qValue.redF(), (float)qValue.greenF(), (float)qValue.blueF(), (float)qValue.alphaF()};
        ParameterModel::setValue(QVariant::fromValue(value));
    }
};
