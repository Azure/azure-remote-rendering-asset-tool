#include <Model/Settings/VideoSettings.h>
#include <ViewModel/Parameters/FloatModel.h>
#include <ViewModel/Parameters/FloatSliderModel.h>
#include <ViewModel/Parameters/IntegerModel.h>
#include <ViewModel/Settings/VideoSettingsModel.h>
#include <string_view>

VideoSettingsModel::VideoSettingsModel(VideoSettings* videoSettings, ArrSessionManager* arrSessionManager, QObject* parent)
    : SettingsBaseModel(parent)
    , m_videoSettings(videoSettings)
    , m_arrSessionManager(arrSessionManager)
{
    using namespace std::literals;
    m_pendingVideoSettings = new VideoSettings(this);
    *m_pendingVideoSettings = *m_videoSettings;

    addControl(new IntegerModel(tr("Horizontal resolution (pixels)"), m_pendingVideoSettings, "width"sv, VideoSettings::s_widthMin, VideoSettings::s_widthMax, VideoSettings::s_resolutionStep));
    addControl(new IntegerModel(tr("Vertical resolution (pixels)"), m_pendingVideoSettings, "height"sv, VideoSettings::s_heightMin, VideoSettings::s_heightMax, VideoSettings::s_resolutionStep));
    addControl(new IntegerModel(tr("Refresh rate (fps)"), m_pendingVideoSettings, "refreshRate"sv, VideoSettings::s_refreshRateMin, VideoSettings::s_refreshRateMax));

    auto updateVideoSettings = [this]() {
        // Video settings can be configured only once before connecting to session runtime.
        // Check that session isn't connecting or connected to runtime
        if (m_arrSessionManager->getSessionStatus().m_status != SessionStatus::Status::ReadyConnecting &&
            m_arrSessionManager->getSessionStatus().m_status != SessionStatus::Status::ReadyConnected)
        {
            *m_videoSettings = *m_pendingVideoSettings;
        }
        Q_EMIT updateUi();
    };

    QObject::connect(m_pendingVideoSettings, &VideoSettings::updateUi, this, &VideoSettingsModel::updateUi);
    QObject::connect(m_pendingVideoSettings, &VideoSettings::changed, this, updateVideoSettings);
    QObject::connect(m_arrSessionManager, &ArrSessionManager::changed, this, [this, updateVideoSettings]() {
        if (m_sessionStatus != m_arrSessionManager->getSessionStatus().m_status)
        {
            m_sessionStatus = m_arrSessionManager->getSessionStatus().m_status;
            updateVideoSettings();
        }
    });
}

bool VideoSettingsModel::canApplyOrResetToCurrentSettings() const
{
    return !(*m_pendingVideoSettings == *m_videoSettings) && m_arrSessionManager->getSessionStatus().m_status == SessionStatus::Status::ReadyConnected;
}

void VideoSettingsModel::applySettings()
{
    *m_videoSettings = *m_pendingVideoSettings;
    m_arrSessionManager->reconnectToSessionRuntime();
    Q_EMIT updateUi();
}

void VideoSettingsModel::resetToCurrentSettings()
{
    *m_pendingVideoSettings = *m_videoSettings;
    Q_EMIT updateUi();
}

bool VideoSettingsModel::isVideoFormatSupported() const
{
    return m_arrSessionManager->getLastError() != RR::Status::VideoFormatNotAvailable;
}
