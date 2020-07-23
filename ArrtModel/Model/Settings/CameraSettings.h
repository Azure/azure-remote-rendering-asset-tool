#pragma once

#include <QObject>

// Model used by the view to configure camera control settings.

class CameraSettings : public QObject
{
    Q_OBJECT

private:
    // Qt reflected properties, wrapping each field in Config
    Q_PROPERTY(float fovAngle MEMBER m_fovAngle NOTIFY changed);
    Q_PROPERTY(float cameraInertia MEMBER m_cameraInertia NOTIFY changed);
    Q_PROPERTY(float cameraSpeed MEMBER m_cameraSpeed NOTIFY changed);
    Q_PROPERTY(float cameraRotationSpeed MEMBER m_cameraRotationSpeed NOTIFY changed);
    Q_PROPERTY(float nearPlane MEMBER m_nearPlane NOTIFY changed);
    Q_PROPERTY(float farPlane MEMBER m_farPlane NOTIFY changed);

public:
    CameraSettings(QObject* parent);

    double getFovAngle() const { return m_fovAngle; }
    float getCameraInertia() const { return m_cameraInertia; }
    float getCameraSpeed() const { return m_cameraSpeed; }
    float getCameraRotationSpeed() const { return m_cameraRotationSpeed; }
    float getNearPlane() const { return m_nearPlane; }
    float getFarPlane() const { return m_farPlane; }

    void loadFromJson(const QJsonObject& cameraConfig);
    QJsonObject saveToJson() const;

Q_SIGNALS:
    void changed();

public:
    static constexpr double s_fovAngleMin = 30.0;
    static constexpr double s_fovAngleMax = 160.0;
    static constexpr float s_cameraInertiaMin = 0.0f;
    static constexpr float s_cameraInertiaMax = 1.0f;
    static constexpr float s_cameraSpeedMin = 0.001f;
    static constexpr float s_cameraSpeedMax = 1000.0f;
    static constexpr float s_cameraRotationSpeedMin = 0.001f;
    static constexpr float s_cameraRotationSpeedMax = 1000.0f;
    static constexpr float s_planeMin = 0.001f;
    static constexpr float s_planeMax = 10000.0f;

private:
    double m_fovAngle = 90.0;
    // Inertia effect is in range from 0.0 to 1.0
    // Set value greater than zero to have inertia effect,
    // if value is zero or less then there is no inertia effect
    float m_cameraInertia = 0.5f;
    float m_cameraSpeed = 2.5f;
    float m_cameraRotationSpeed = 0.5f;
    float m_nearPlane = 0.1f;
    float m_farPlane = 100.f;
};
