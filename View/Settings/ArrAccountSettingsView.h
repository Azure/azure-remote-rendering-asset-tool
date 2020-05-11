#pragma once
#include <View/Settings/SettingsBaseView.h>

class ArrAccountSettingsModel;
class FlatButton;

// View for ARR account settings configuration.

class ArrAccountSettingsView : public SettingsBaseView
{
public:
    ArrAccountSettingsView(ArrAccountSettingsModel* model, QWidget* parent = nullptr);

private:
    ArrAccountSettingsModel* const m_model;
    FlatButton* m_retryButton = {};

    void updateUi();
};
