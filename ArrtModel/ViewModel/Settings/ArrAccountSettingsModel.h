#pragma once

#include <Model/Settings/AccountConnectionStatus.h>
#include <ViewModel/Settings/SettingsBaseModel.h>

class ArrFrontend;
class ArrAccountSettings;

class ArrAccountSettingsModel : public SettingsBaseModel
{
public:
    ArrAccountSettingsModel(ArrAccountSettings* arrAccountSettings, ArrFrontend* frontend, QObject* parent);

    bool isEnabled() const override;
    AccountConnectionStatus getStatus() const;

    void reconnectAccount();

private:
    ArrAccountSettings* const m_arrAccountSettings;
    ArrFrontend* const m_frontend;
};
