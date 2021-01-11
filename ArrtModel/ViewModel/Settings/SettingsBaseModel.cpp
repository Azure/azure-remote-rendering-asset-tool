#include <ViewModel/Settings/SettingsBaseModel.h>
#include <ViewModel/Parameters/ParameterModel.h>

SettingsBaseModel::SettingsBaseModel(QObject* parent)
    : QObject(parent)
{
}

void SettingsBaseModel::addControl(ParameterModel* model)
{
    model->setParent(this);
    m_controls.push_back(model);
}
