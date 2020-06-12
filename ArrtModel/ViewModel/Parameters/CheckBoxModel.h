#pragma once
#include <QVariant>
#include <ViewModel/Parameters/ParameterModel.h>

// Model for a boolean parameter

class CheckBoxModel : public ParameterModel
{
    Q_OBJECT

public:
    CheckBoxModel(QString name, QObject* object, const std::string_view& parameterName)
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
