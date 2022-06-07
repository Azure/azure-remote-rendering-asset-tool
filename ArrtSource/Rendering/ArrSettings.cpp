#include "ArrSettings.h"
#include <QSettings>

void ArrSettings::SetVideoWidth(int width)
{
    width = std::clamp(width, s_videoWidthMin, s_videoWidthMax);

    if (m_videoWidth != width)
    {
        m_videoWidth = width;
        Q_EMIT OptionsChanged();
    }
}

void ArrSettings::SetVideoHeight(int height)
{
    height = std::clamp(height, s_videoHeightMin, s_videoHeightMax);

    if (m_videoHeight != height)
    {
        m_videoHeight = height;
        Q_EMIT OptionsChanged();
    }
}

void ArrSettings::SetVideoRefreshRate(int r)
{
    if (m_videoRefreshRate != r)
    {
        m_videoRefreshRate = r;
        Q_EMIT OptionsChanged();
    }
}

void ArrSettings::SetFovAngle(int value)
{
    if (m_fovAngle != value)
    {
        m_fovAngle = value;
        Q_EMIT OptionsChanged();
    }
}

void ArrSettings::SetCameraSpeedInPow2(int value)
{
    if (m_cameraSpeedPow2 != value)
    {
        m_cameraSpeedPow2 = value;
        Q_EMIT OptionsChanged();
    }
}

float ArrSettings::GetCameraSpeedMetersPerSecond() const
{
    return powf(2.0f, (float)m_cameraSpeedPow2) / 100.0f;
}

void ArrSettings::SetNearPlaneCM(int value)
{
    if (m_nearPlaneCM != value)
    {
        m_nearPlaneCM = value;
        Q_EMIT OptionsChanged();
    }
}

void ArrSettings::SetFarPlaneCM(int value)
{
    if (m_farPlaneCM != value)
    {
        m_farPlaneCM = value;
        Q_EMIT OptionsChanged();
    }
}

void ArrSettings::SetPointSize(int value)
{
    if (m_pointSize != value)
    {
        m_pointSize = value;
        Q_EMIT OptionsChanged();
    }
}

void ArrSettings::SaveSettings() const
{
    QSettings s;
    s.beginGroup("Video");
    s.setValue("VideoWidth", m_videoWidth);
    s.setValue("VideoHeight", m_videoHeight);
    s.setValue("VideoRefreshRate", m_videoRefreshRate);
    s.setValue("VideoFOV", m_fovAngle);
    s.setValue("CameraNear", m_nearPlaneCM);
    s.setValue("CameraFar", m_farPlaneCM);
    s.setValue("CameraSpeed", m_cameraSpeedPow2);
    s.setValue("PointSize", m_pointSize);
    s.endGroup();
}

void ArrSettings::LoadSettings()
{
    QSettings s;
    s.beginGroup("Video");
    m_videoWidth = s.value("VideoWidth", m_videoWidth).toInt();
    m_videoHeight = s.value("VideoHeight", m_videoHeight).toInt();
    m_videoRefreshRate = s.value("VideoRefreshRate", m_videoRefreshRate).toInt();
    m_fovAngle = s.value("VideoFOV", m_fovAngle).toInt();
    m_nearPlaneCM = s.value("CameraNear", m_nearPlaneCM).toInt();
    m_farPlaneCM = s.value("CameraFar", m_farPlaneCM).toInt();
    m_cameraSpeedPow2 = s.value("CameraSpeed", m_cameraSpeedPow2).toInt();
    m_pointSize = s.value("PointSize", m_pointSize).toInt();
    s.endGroup();
}
