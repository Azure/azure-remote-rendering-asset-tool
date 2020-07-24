#include <Model/Settings/CameraSettings.h>
#include <QtMath>
#include <Utils/JsonUtils.h>

CameraSettings::CameraSettings(QObject* parent)
    : QObject(parent)
{
}

void CameraSettings::setGlobalScaleAutomatic(bool globalScaleAutomatic)
{
    m_autoGlobalScale = globalScaleAutomatic;
    Q_EMIT scaleChanged();
    Q_EMIT changed();
}

void CameraSettings::setGlobalScale(float globalScale)
{
    m_globalScale = std::clamp(globalScale, s_scaleMin, s_scaleMax);
    Q_EMIT scaleChanged();
    Q_EMIT changed();
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

    m_cameraRotationSpeed = JsonUtils::fromJson(cameraConfig, QLatin1String("cameraRotationSpeed"), m_cameraRotationSpeed);
    m_cameraRotationSpeed = std::clamp(m_cameraRotationSpeed, s_cameraRotationSpeedMin, s_cameraRotationSpeedMax);

    m_nearPlane = JsonUtils::fromJson(cameraConfig, QLatin1String("nearPlane"), m_nearPlane);
    m_nearPlane = std::clamp(m_nearPlane, s_planeMin, s_planeMax);

    m_farPlane = JsonUtils::fromJson(cameraConfig, QLatin1String("farPlane"), m_farPlane);
    m_farPlane = std::clamp(m_farPlane, s_planeMin, s_planeMax);

    m_globalScale = JsonUtils::fromJson(cameraConfig, QLatin1String("globalScale"), m_globalScale);
    m_globalScale = std::clamp(m_globalScale, s_scaleMin, s_scaleMax);

    m_autoGlobalScale = JsonUtils::fromJson(cameraConfig, QLatin1String("autoGlobalScale"), m_autoGlobalScale);
}

QJsonObject CameraSettings::saveToJson() const
{
    QJsonObject cameraConfig;
    cameraConfig[QLatin1String("fovangle")] = m_fovAngle;
    cameraConfig[QLatin1String("cameraInertia")] = m_cameraInertia;
    cameraConfig[QLatin1String("cameraSpeed")] = m_cameraSpeed;
    cameraConfig[QLatin1String("cameraRotationSpeed")] = m_cameraRotationSpeed;
    cameraConfig[QLatin1String("nearPlane")] = m_nearPlane;
    cameraConfig[QLatin1String("farPlane")] = m_farPlane;
    cameraConfig[QLatin1String("globalScale")] = m_globalScale;
    cameraConfig[QLatin1String("autoGlobalScale")] = m_autoGlobalScale;
    return cameraConfig;
}
