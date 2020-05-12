#pragma once
#include <Model/IncludesAzureRemoteRendering.h>
#include <ViewModel/ModelEditor/MaterialModel.h>

class ArrSessionManager;

// MaterialModel implementation for a PBR material, wrapping a RR::PbrMaterial

class MaterialPBR : public MaterialModel
{
    Q_OBJECT

    typedef RR::PbrMaterialFeatures FlagsType;

public:
    MaterialPBR(ArrSessionManager* sessionManager, QObject* parent = nullptr);

    //enums copy pasted from RR::PbrMaterial. This is because moc can't parse enums on non Q_OBJECT marked classes

    /// \brief Defines how the alpha portion of the mesh's vertex colors contribute to the final color. This enum can be passed to SetVertexAlphaMode.
    enum class PbrVertexAlphaMode : int32_t
    {
        Occlusion = 0, ///< The alpha value serves as an intensity for the material's ambient occlusion value.
        LightMask = 1, ///< The alpha value serves as an intensity for the amount of lighting applied, i.e. the alpha can be used to darken areas.
        Opacity = 2    ///< The alpha value affects the level of transparency for transparent materials.
    };

    /// \brief Additional generic material flags as passed to SetFlags or retrieved from GetFlags.
    enum class PbrMaterialFeatures : int32_t
    {
        //NoMaterialFlags = 0,        ///< None of the flags below.
        TransparentMaterial = 1, ///< The material is transparent (alpha-blended), where the level of transparency is defined by albedo colors' alpha and optionally vertex colors' alpha.
        UseVertexColor = 2,      ///< Use/ignore the vertex color (if provided by the mesh). Needs to be enabled so that SetVertexColorMode/SetVertexAlphaMode has any effect.
        DoubleSided = 4,         ///< The material is rendered double-sided, otherwise back faces are culled.
        SpecularEnabled = 8,     ///< Enables specular highlights for this material.
        AlphaClipped = 16,       ///< Enables hard cut-outs on a per-pixel basis based on the alpha value being below a threshold. This works for opaque materials as well.
        FadeToBlack = 32         ///< If enabled, this material fades to black as opposed to fading to transparent when used with SetFadeOut. Fading to black has the same effect
                                 ///< on see-through devices like Hololens but has less GPU cost associated with it.
    };

    //Q_ENUM(VertexColorMode)
    Q_ENUM(PbrVertexAlphaMode)
    Q_ENUM(PbrMaterialFeatures)

    ARRT_PROPERTY(PbrMaterialFeatures, PbrFlags);
    ARRT_PROPERTY(RR::Float2, TexCoordScale);
    ARRT_PROPERTY(RR::Float2, TexCoordOffset);
    ARRT_PROPERTY(RR::Color4, AlbedoColor);
    ARRT_PROPERTY(RR::ApiHandle<RR::Texture>, AlbedoTexture);
    ARRT_PROPERTY(PbrVertexAlphaMode, PbrVertexAlphaMode);
    ARRT_PROPERTY(RR::ApiHandle<RR::Texture>, NormalMap);
    ARRT_PROPERTY(float, AOScale);
    ARRT_PROPERTY(RR::ApiHandle<RR::Texture>, AOMap);
    ARRT_PROPERTY(float, Roughness);
    ARRT_PROPERTY(RR::ApiHandle<RR::Texture>, RoughnessMap);
    ARRT_PROPERTY(float, Metalness);
    ARRT_PROPERTY(RR::ApiHandle<RR::Texture>, MetalnessMap);
    ARRT_PROPERTY(float, AlphaClipThreshold);
    ARRT_PROPERTY(float, FadeOut);

private:
    const RR::ApiHandle<RR::PbrMaterial> getMaterial() const;
    RR::ApiHandle<RR::PbrMaterial> getMaterial();
};
