#include <Model/Settings/CameraSettings.h>
#include <Utils/JsonUtils.h>

CameraSettings::CameraSettings(QObject* parent)
    : QObject(parent)
{
}

void CameraSettings::loadFromJson(const QJsonObject& cameraConfig)
{
    m_fovAngle = JsonUtils::fromJson(cameraConfig, QLatin1String("fovangle"), m_fovAngle);
    m_fovAngle = std::clamp(m_fovAngle, s_fovAngleMin, s_fovAngleMax);

    m_cameraInertia = JsonUtils::fromJson(cameraConfig, QLatin1String("cameraInertia"), m_cameraInertia);
    m_cameraInertia = std::clamp(m_cameraInertia, s_cameraInertiaMin, s_cameraInertiaMax);

    m_cameraSpeed = JsonUtils::fromJson(cameraConfig, QLatin1String("cameraSpeed"), m_cameraSpeed);
    m_cameraSpeed = std::clamp(m_cameraSpeed, s_cameraSpeedMin, s_cameraSpeedMax);

    m_cameraRotationSpeed = JsonUtils::fromJson(cameraConfig, QLatin1String("cameraRotationSpeed"), m_cameraRotationSpeed);
    m_cameraRotationSpeed = std::clamp(m_cameraRotationSpeed, s_cameraRotationSpeedMin, s_cameraRotationSpeedMax);
}

QJsonObject CameraSettings::saveToJson() const
{
    QJsonObject cameraConfig;
    cameraConfig[QLatin1String("fovangle")] = m_fovAngle;
    cameraConfig[QLatin1String("cameraInertia")] = m_cameraInertia;
    cameraConfig[QLatin1String("cameraSpeed")] = m_cameraSpeed;
    cameraConfig[QLatin1String("cameraRotationSpeed")] = m_cameraRotationSpeed;
    return cameraConfig;
}
