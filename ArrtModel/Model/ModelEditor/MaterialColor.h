#pragma once
#include <Model/IncludesAzureRemoteRendering.h>
#include <ViewModel/ModelEditor/MaterialEditor/MaterialModel.h>

class ArrSessionManager;

// MaterialModel implementation for a Color material, wrapping a RR::ColorMaterial

class MaterialColor : public MaterialModel
{
    Q_OBJECT

public:
    MaterialColor(ArrSessionManager* sessionManager, QObject* parent = nullptr);

    //enums copy pasted from RR::ColorMaterial. This is because moc can't parse enums on non Q_OBJECT marked classes

    /// \brief Defines how the alpha portion of the mesh's vertex colors contribute to the final color. This enum can be passed to SetVertexAlphaMode.
    enum class ColorTransparencyMode : int32_t
    {
        Opaque = 0, ///< The material is opaque, but still allows for hard cutouts when the Flags::AlphaClipped flag is specified.
        AlphaBlend, ///< The material is semi-transparent through alpha-blending, using the combined albedo's alpha for opacity.
        Additive    ///< The material is blended additively.
    };

    /// \brief Additional generic material flags as passed to SetFlags or retrieved from GetFlags.
    enum class ColorMaterialFeatures : int32_t
    {
        // NoMaterialFlags = 0, ///< None of the flags below.
        UseVertexColor = 1, ///< Use/ignore the vertex color if provided by the mesh.
        DoubleSided = 2,    ///< The material is rendered double-sided, otherwise back faces are culled.
        FadeToBlack = 4,    ///< If enabled, this material fades to black as opposed to fading to transparent when used with SetFadeOut. Fading to black has the same effect
                            ///< on see-through devices like Hololens but has less GPU cost associated with it.
        AlphaClipped = 8    ///< Enables hard cut-outs on a per-pixel basis based on the alpha value being below a threshold. This works for opaque materials as well.
    };

    Q_ENUM(ColorMaterialFeatures)
    Q_ENUM(ColorTransparencyMode)

    ARRT_PROPERTY(ColorMaterialFeatures, ColorFlags);
    ARRT_PROPERTY(RR::Color4, AlbedoColor);
    ARRT_PROPERTY(RR::ApiHandle<RR::Texture>, AlbedoTexture);
    ARRT_PROPERTY(RR::Float2, TexCoordScale);
    ARRT_PROPERTY(RR::Float2, TexCoordOffset);
    ARRT_PROPERTY(ColorTransparencyMode, ColorTransparencyMode);
    ARRT_PROPERTY(float, FadeOut);
    ARRT_PROPERTY(float, VertexMix);
    ARRT_PROPERTY(float, AlphaClipThreshold);

private:
    const RR::ApiHandle<RR::ColorMaterial> getMaterial() const;
    RR::ApiHandle<RR::ColorMaterial> getMaterial();
};
