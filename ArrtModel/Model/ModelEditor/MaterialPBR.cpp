#include <Model/ArrSessionManager.h>
#include <Model/ModelEditor/MaterialPBR.h>
#include <ViewModel/Parameters/ColorModel.h>
#include <ViewModel/Parameters/ComboBoxModel.h>
#include <ViewModel/Parameters/FloatModel.h>
#include <ViewModel/Parameters/FloatSliderModel.h>
#include <ViewModel/Parameters/FloatVectorModel.h>
#include <ViewModel/Parameters/MultiComboBoxModel.h>
#include <ViewModel/Parameters/TextureModel.h>
#include <string_view>

MaterialPBR::MaterialPBR(ArrSessionManager* sessionManager, QObject* parent)
    : MaterialModel(sessionManager, parent)
{
    using namespace std::literals;
    m_controls.push_back(new MultiComboBoxModel(tr("Flags"), this, "PbrFlags"sv));
    m_controls.push_back(new Float2Model(tr("Texture Scale"), this, "TexCoordScale"sv));
    m_controls.push_back(new Float2Model(tr("Texture Offset"), this, "TexCoordOffset"sv));
    m_controls.push_back(new ColorModel(tr("Albedo Color"), this, "AlbedoColor"sv));
    m_controls.push_back(new TextureModel(tr("Albedo Texture"), this, "AlbedoTexture"sv));
    m_controls.push_back(new ComboBoxModel(tr("Vertex Alpha Mode"), this, "PbrVertexAlphaMode"sv));
    m_controls.push_back(new TextureModel(tr("Normal Map"), this, "NormalMap"sv));
    m_controls.push_back(new FloatSliderModel(tr("Ambient Occlusion Scale"), this, "AOScale"sv, 0.0f, 1.0f, 1000));
    m_controls.push_back(new TextureModel(tr("Ambient Occlusion Map"), this, "AOMap"sv));
    m_controls.push_back(new FloatSliderModel(tr("Roughness"), this, "Roughness"sv, 0.0f, 1.0f, 1000));
    m_controls.push_back(new TextureModel(tr("Roughness Map"), this, "RoughnessMap"sv));
    m_controls.push_back(new FloatSliderModel(tr("Metalness"), this, "Metalness"sv, 0.0f, 1.0f, 1000));
    m_controls.push_back(new TextureModel(tr("Metalness Map"), this, "MetalnessMap"sv));
    m_controls.push_back(new FloatSliderModel(tr("Alpha Clip Threshold"), this, "AlphaClipThreshold"sv, 0.0f, 1.0f, 1000));
    m_controls.push_back(new FloatSliderModel(tr("Fade Out"), this, "FadeOut"sv, 0.0f, 1.0f, 1000));
}

RR::ApiHandle<RR::PbrMaterial> MaterialPBR::getMaterial()
{
    if (m_material)
    {
        if (auto type = m_material->MaterialSubType())
        {
            if (type.value() == RR::MaterialType::Pbr)
            {
                return m_material.as<RR::PbrMaterial>();
            }
        }
    }
    return {};
}


const RR::ApiHandle<RR::PbrMaterial> MaterialPBR::getMaterial() const
{
    if (m_material && *m_material->MaterialSubType() == RR::MaterialType::Pbr)
    {
        if (auto type = m_material->MaterialSubType())
        {
            if (type.value() == RR::MaterialType::Pbr)
            {
                auto material = m_material;
                return material.as<RR::PbrMaterial>();
            }
        }
    }
    return {};
}
