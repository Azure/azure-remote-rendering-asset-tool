#pragma once

#include <ViewModel/Settings/SettingsBaseModel.h>

class CameraSettings;
class ToggleButtonModel;
class FloatSliderModel;
class ToggleButtonModel;

// Model used by the view to configure camera control settings.

class CameraSettingsModel : public SettingsBaseModel
{
    Q_OBJECT

public:
    CameraSettingsModel(CameraSettings* cameraSettings, QObject* parent);

    bool isEnabled() const override { return true; }
    FloatSliderModel* getGlobalScaleModel() const { return m_globalScaleModel; }
    ToggleButtonModel* getAutoGlobalScaleModel() const { return m_autoGlobalScaleModel; }

Q_SIGNALS:
    void scaleChanged();

private:
    CameraSettings* const m_cameraSettings;
    FloatSliderModel* m_globalScaleModel = {};
    ToggleButtonModel* m_autoGlobalScaleModel = {};
};
