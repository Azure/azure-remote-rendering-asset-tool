#include <Model/ArrSessionManager.h>
#include <Model/ModelEditor/MaterialColor.h>
#include <ViewModel/Parameters/ColorModel.h>
#include <ViewModel/Parameters/ComboBoxModel.h>
#include <ViewModel/Parameters/FloatModel.h>
#include <ViewModel/Parameters/FloatSliderModel.h>
#include <ViewModel/Parameters/FloatVectorModel.h>
#include <ViewModel/Parameters/MultiComboBoxModel.h>
#include <ViewModel/Parameters/TextureModel.h>
#include <string_view>

MaterialColor::MaterialColor(ArrSessionManager* sessionManager, QObject* parent)
    : MaterialModel(sessionManager, parent)
{
    using namespace std::literals;

    m_controls.push_back(new MultiComboBoxModel(tr("Flags"), this, "ColorFlags"sv));
    m_controls.push_back(new ColorModel(tr("Albedo Color"), this, "AlbedoColor"sv));
    m_controls.push_back(new TextureModel(tr("Albedo Texture"), this, "AlbedoTexture"sv));
    m_controls.push_back(new Float2Model(tr("Texture Scale"), this, "TexCoordScale"sv));
    m_controls.push_back(new Float2Model(tr("Texture Offset"), this, "TexCoordOffset"sv));
    m_controls.push_back(new ComboBoxModel(tr("Color Transparency Mode"), this, "ColorTransparencyMode"sv));
    m_controls.push_back(new FloatSliderModel(tr("Fade Out"), this, "FadeOut"sv, 0.0f, 1.0f, 1000));
    m_controls.push_back(new FloatSliderModel(tr("Vertex Mix"), this, "VertexMix"sv, 0.0f, 1.0f, 1000));
    m_controls.push_back(new FloatSliderModel(tr("Alpha Clip Threshold"), this, "AlphaClipThreshold"sv, 0.0f, 1.0f, 1000));
}

RR::ApiHandle<RR::ColorMaterial> MaterialColor::getMaterial()
{
    if (m_material)
    {
        if (auto type = m_material->MaterialSubType())
        {
            if (type.value() == RR::MaterialType::Color)
            {
                return m_material.as<RR::ColorMaterial>();
            }
        }
    }
    return {};
}


const RR::ApiHandle<RR::ColorMaterial> MaterialColor::getMaterial() const
{
    if (m_material && *m_material->MaterialSubType() == RR::MaterialType::Color)
    {
        if (auto type = m_material->MaterialSubType())
        {
            if (type.value() == RR::MaterialType::Color)
            {
                auto material = m_material;
                return material.as<RR::ColorMaterial>();
            }
        }
    }
    return {};
}
