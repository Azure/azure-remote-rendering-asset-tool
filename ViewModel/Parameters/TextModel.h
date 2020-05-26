#pragma once
#include <QVariant>
#include <ViewModel/Parameters/ParameterModel.h>

// model for a BoundText, over a QString property

class TextModel : public ParameterModel
{
    Q_OBJECT

public:
    TextModel(QString name, QObject* object, const std::string_view& parameterName, bool updateOnEditingFinished = false, bool isPassword = false)
        : ParameterModel(name, object, parameterName)
        , m_updateOnEditingFinished(updateOnEditingFinished)
        , m_isPassword(isPassword)
    {
    }

    QString getValue() const
    {
        return ParameterModel::getValue().value<QString>();
    }
    void setValue(QString value)
    {
        ParameterModel::setValue(QVariant::fromValue<QString>(value));
    }

    const bool m_updateOnEditingFinished;
    const bool m_isPassword;
};
