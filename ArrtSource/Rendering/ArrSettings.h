#pragma once

#include <QObject>

/// Some general settings (camera speed and such) that are persisted between sessions.
class ArrSettings : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void OptionsChanged();

public:
    int GetVideoWidth() const { return m_videoWidth; }
    void SetVideoWidth(int w);

    int GetVideoHeight() const { return m_videoHeight; }
    void SetVideoHeight(int h);

    int GetVideoRefreshRate() const { return m_videoRefreshRate; }
    void SetVideoRefreshRate(int r);

    int GetFovAngle() const { return m_fovAngle; }
    void SetFovAngle(int value);

    int GetCameraSpeedInPow2() const { return m_cameraSpeedPow2; }
    void SetCameraSpeedInPow2(int value);

    float GetCameraSpeedMetersPerSecond() const;

    int GetNearPlaneCM() const { return m_nearPlaneCM; }
    void SetNearPlaneCM(int value);

    int GetFarPlaneCM() const { return m_farPlaneCM; }
    void SetFarPlaneCM(int value);

    int GetPointSize() const { return m_pointSize; }
    void SetPointSize(int value);

    void SaveSettings() const;
    void LoadSettings();

    static constexpr int s_videoWidthMin = 256;
    static constexpr int s_videoWidthMax = 4096;

    static constexpr int s_videoHeightMin = 256;
    static constexpr int s_videoHeightMax = 2160;

    static constexpr int s_videoResolutionStep = 8;

    static constexpr int s_videoRefreshRateMin = 30;
    static constexpr int s_videoRefreshRateMax = 60;

private:
    int m_videoWidth = 1440;
    int m_videoHeight = 936;
    int m_videoRefreshRate = 60;

    int m_fovAngle = 90;
    int m_cameraSpeedPow2 = 6; // 64 cm per second

    int m_nearPlaneCM = 5;
    int m_farPlaneCM = 5000;

    int m_pointSize = 10;
};