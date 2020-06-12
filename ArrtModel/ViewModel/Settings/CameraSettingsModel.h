#pragma once

#include <ViewModel/Settings/SettingsBaseModel.h>

class CameraSettings;

// Model used by the view to configure camera control settings.

class CameraSettingsModel : public SettingsBaseModel
{
public:
    CameraSettingsModel(CameraSettings* cameraSettings, QObject* parent);

    bool isEnabled() const override { return true; }

private:
    CameraSettings* m_cameraSettings;
};
