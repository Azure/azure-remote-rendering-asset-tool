#pragma once
#include <QVariant>
#include <ViewModel/Parameters/ParameterModel.h>

// model for a toggle button, over a boolean property

class ToggleButtonModel : public ParameterModel
{
    Q_OBJECT

public:
    ToggleButtonModel(QString name, QObject* object, const std::string_view& parameterName)
        : ParameterModel(name, object, parameterName)
    {
    }

    bool getValue() const
    {
        return ParameterModel::getValue().value<bool>();
    }
    void setValue(bool value)
    {
        ParameterModel::setValue(QVariant::fromValue<bool>(value));
    }
};
