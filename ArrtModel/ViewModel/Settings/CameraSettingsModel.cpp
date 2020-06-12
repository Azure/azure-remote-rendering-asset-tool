#include <Model/Settings/CameraSettings.h>
#include <ViewModel/Parameters/FloatSliderModel.h>
#include <ViewModel/Settings/CameraSettingsModel.h>
#include <string_view>

CameraSettingsModel::CameraSettingsModel(CameraSettings* cameraSettings, QObject* parent)
    : SettingsBaseModel(parent)
    , m_cameraSettings(cameraSettings)
{
    using namespace std::literals;
    m_controls.push_back(new FloatSliderModel(tr("Field of view (degrees)"), m_cameraSettings, "fovAngle"sv, CameraSettings::s_fovAngleMin, CameraSettings::s_fovAngleMax, 1000));
    m_controls.push_back(new FloatSliderModel(tr("Camera inertia"), m_cameraSettings, "cameraInertia"sv, CameraSettings::s_cameraInertiaMin, CameraSettings::s_cameraInertiaMax, 256));
    m_controls.push_back(new FloatSliderModel(tr("Camera speed"), m_cameraSettings, "cameraSpeed"sv, CameraSettings::s_cameraRotationSpeedMin, CameraSettings::s_cameraSpeedMax, 10000));
    m_controls.push_back(new FloatSliderModel(tr("Camera rotation speed"), m_cameraSettings, "cameraRotationSpeed"sv, CameraSettings::s_cameraRotationSpeedMin, CameraSettings::s_cameraRotationSpeedMax, 10000));
}
