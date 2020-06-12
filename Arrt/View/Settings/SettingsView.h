#pragma once
#include <QWidget>

class SettingsModel;

// View with Azure Storage account and ARR account settings configurations.

class SettingsView : public QWidget
{
public:
    SettingsView(SettingsModel* settingsModel, QWidget* parent = nullptr);
};
