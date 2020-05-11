#pragma once

#include <QVariant>
#include <ViewModel/Parameters/ParameterModel.h>

// model for an texture parameter, using RR::ObjectIdAndType as the backend

class TextureModel : public ParameterModel
{
    Q_OBJECT

public:
    TextureModel(QString name, QObject* object, const std::string_view& parameterName)
        : ParameterModel(name, object, parameterName)
    {
    }

    QString getValue() const
    {
        return ""; // <TODO>
    }
    void setValue(float /*value*/)
    {
        // <TODO>
    }
};
