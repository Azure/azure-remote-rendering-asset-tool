#pragma once

#include <Model/ArrSessionManager.h>
#include <Model/Settings/AccountConnectionStatus.h>
#include <ViewModel/Settings/SettingsBaseModel.h>

class ParameterModel;
class VideoSettings;

// Model used by the view to configure ARR rendering session video settings.

class VideoSettingsModel : public SettingsBaseModel
{
public:
    VideoSettingsModel(VideoSettings* videoSettings, ArrSessionManager* arrSessionManager, QObject* parent);

    bool isEnabled() const override { return true; }
    bool canApplyOrResetToCurrentSettings() const;
    void applySettings();
    void resetToCurrentSettings();

    bool isVideoFormatSupported() const;

private:
    VideoSettings* const m_videoSettings = {};
    VideoSettings* m_pendingVideoSettings = {};

    ArrSessionManager* m_arrSessionManager = {};
    SessionStatus::Status m_sessionStatus = {};
};
