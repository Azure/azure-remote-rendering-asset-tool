#pragma once
#include <QLineEdit>
#include <QWidget>

#include <View/Settings/SettingsBaseView.h>

class StorageAccountSettingsModel;
class FlatButton;

// View for Azure Storage account settings configuration.

class StorageAccountSettingsView : public SettingsBaseView
{
public:
    StorageAccountSettingsView(StorageAccountSettingsModel* model, QWidget* parent = nullptr);

private:
    StorageAccountSettingsModel* const m_model;
    FlatButton* m_retryButton = {};

    void updateUi();
};
