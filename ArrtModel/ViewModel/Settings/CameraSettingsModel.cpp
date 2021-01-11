#include <Model/Settings/CameraSettings.h>
#include <ViewModel/Parameters/FloatSliderModel.h>
#include <ViewModel/Parameters/ToggleButtonModel.h>
#include <ViewModel/Settings/CameraSettingsModel.h>
#include <string_view>

CameraSettingsModel::CameraSettingsModel(CameraSettings* cameraSettings, QObject* parent)
    : SettingsBaseModel(parent)
    , m_cameraSettings(cameraSettings)
{
    using namespace std::literals;
    addControl(new FloatSliderModel(tr("Field of view (degrees)"), m_cameraSettings, "fovAngle"sv, CameraSettings::s_fovAngleMin, CameraSettings::s_fovAngleMax, 1000));
    addControl(new FloatSliderModel(tr("Camera inertia"), m_cameraSettings, "cameraInertia"sv, CameraSettings::s_cameraInertiaMin, CameraSettings::s_cameraInertiaMax, 256));
    addControl(new FloatSliderModel(tr("Camera speed"), m_cameraSettings, "cameraSpeed"sv, CameraSettings::s_cameraRotationSpeedMin, CameraSettings::s_cameraSpeedMax, 10000, true));
    addControl(new FloatSliderModel(tr("Camera rotation speed"), m_cameraSettings, "cameraRotationSpeed"sv, CameraSettings::s_cameraRotationSpeedMin, CameraSettings::s_cameraRotationSpeedMax, 10000, true));
    addControl(new FloatSliderModel(tr("Near plane"), m_cameraSettings, "nearPlane"sv, CameraSettings::s_planeMin, CameraSettings::s_planeMax, 10000, true, "%.5fm"));
    addControl(new FloatSliderModel(tr("Far plane"), m_cameraSettings, "farPlane"sv, CameraSettings::s_planeMin, CameraSettings::s_planeMax, 10000, true, "%.5fm"));

    m_globalScaleModel = new FloatSliderModel(tr("Global scale"), m_cameraSettings, "globalScale"sv, CameraSettings::s_scaleMin, CameraSettings::s_scaleMax, 10000, true, "%.5fm");
    m_autoGlobalScaleModel = new ToggleButtonModel(tr("Auto"), m_cameraSettings, "autoGlobalScale"sv);

    connect(m_cameraSettings, &CameraSettings::scaleChanged, this, &CameraSettingsModel::scaleChanged);
}
