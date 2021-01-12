#include <ViewModel/Parameters/ParameterModel.h>
#include <ViewModel/Settings/SettingsBaseModel.h>

SettingsBaseModel::SettingsBaseModel(QObject* parent)
    : QObject(parent)
{
}

void SettingsBaseModel::addControl(ParameterModel* model)
{
    model->setParent(this);
    m_controls.push_back(model);
}
