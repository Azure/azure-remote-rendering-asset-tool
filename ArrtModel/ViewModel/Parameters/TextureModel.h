#pragma once

#include <Model/IncludesAzureRemoteRendering.h>
#include <QVariant>
#include <ViewModel/Parameters/ParameterModel.h>

// model for an texture parameter, using RR::ApiHandle<RR::Texture> as the backend

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
        auto val = ParameterModel::getValue().value<RR::ApiHandle<RR::Texture>>();
        if (val.valid())
        {
            std::string name;
            val->GetName(name);
            //remove the part between :{}
            if (name._Starts_with(":{"))
            {
                auto lastToRemove = name.find("}");
                if (lastToRemove != -1)
                {
                    name = name.substr(lastToRemove + 1);
                }
            }
            return QString::fromStdString(name);
        }
        else
        {
            return tr("Not set");
        }
    }
};
