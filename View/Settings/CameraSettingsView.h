#pragma once
#include <View/Settings/SettingsBaseView.h>

class CameraSettingsModel;

// View for camera settings configuration.

class CameraSettingsView : public SettingsBaseView
{
public:
    CameraSettingsView(CameraSettingsModel* model, QWidget* parent = nullptr);
};
