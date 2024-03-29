#pragma once

#include <QDateTime>
#include <QString>

enum class ConversionStatus
{
    New,
    Running,
    Finished,
    Failed,
};

enum class Sideness
{
    SingleSided = 0,
    DoubleSided
};

enum class Axis
{
    Inherit = 0,
    PosX,
    NegX,
    PosY,
    NegY,
    PosZ,
    NegZ
};

enum class SceneGraphMode
{
    None = 0,
    Static,
    Dynamic
};

enum class VertexPosition
{
    FormatDefault,
    Float32x3,
    Float16x3
};

enum class VertexColor
{
    FormatDefault,
    None,
    ByteUx4N
};

enum class VertexVector
{
    FormatDefault,
    None,
    ByteSx4N,
    Float16x4,
};

enum class VertexTextureCoord
{
    FormatDefault,
    None,
    Float32x2,
    Float16x2
};

enum class ColorSpaceMode
{
    FormatDefault,
    GammaSpace,
    LinearSpace,
};

enum class ConversionOption : uint64_t
{
    UniformScaling = 1 << 0,
    RecenterToOrigin = 1 << 1,
    MaterialDefaultSidedness = 1 << 2,
    GammaToLinearVertex = 1 << 3,
    GammaToLinearMaterial = 1 << 4,
    CollisionMesh = 1 << 5,
    UnlitMaterials = 1 << 6,
    FbxAssumeMetallic = 1 << 7,
    DeduplicateMaterials = 1 << 8,
    AxisMapping = 1 << 9,
    VertexPositionFormat = 1 << 10,
    VertexColor0Format = 1 << 11,
    VertexColor1Format = 1 << 12,
    VertexNormalFormat = 1 << 13,
    VertexTangentFormat = 1 << 14,
    VertexBinormalFormat = 1 << 15,
    VertexTexCoord0Format = 1 << 16,
    VertexTexCoord1Format = 1 << 17,
    SceneGraphMode = 1 << 18,

    All = 0xFFFFFFFFFFFFFFFF
};

/// All options describing a conversion
struct ConversionOptions
{
    float m_scaling = 1.0f;
    bool m_recenterToOrigin = false;
    Sideness m_opaqueMaterialDefaultSidedness = Sideness::DoubleSided;
    ColorSpaceMode m_materialColorSpace = ColorSpaceMode::FormatDefault;
    ColorSpaceMode m_vertexColorSpace = ColorSpaceMode::FormatDefault;
    SceneGraphMode m_sceneGraphMode = SceneGraphMode::Static;
    bool m_generateCollisionMesh = true;
    bool m_unlitMaterials = false;
    bool m_fbxAssumeMetallic = true;
    bool m_deduplicateMaterials = true;
    Axis m_axis1 = Axis::PosX;
    Axis m_axis2 = Axis::PosY;
    Axis m_axis3 = Axis::PosZ;
    VertexPosition m_vertexPosition = VertexPosition::FormatDefault;
    VertexColor m_vertexColor0 = VertexColor::FormatDefault;
    VertexColor m_vertexColor1 = VertexColor::FormatDefault;
    VertexVector m_vertexNormal = VertexVector::FormatDefault;
    VertexVector m_vertexTangent = VertexVector::FormatDefault;
    VertexVector m_vertexBinormal = VertexVector::FormatDefault;
    VertexTextureCoord m_vertexTexCoord0 = VertexTextureCoord::FormatDefault;
    VertexTextureCoord m_vertexTexCoord1 = VertexTextureCoord::FormatDefault;

    QString ToJSON(uint64_t availableOptions) const;
};

/// A single conversion that either ran previously or is currently running
struct Conversion
{
    ConversionStatus m_status = ConversionStatus::New;

    QString m_name;
    QString m_sourceAssetContainer;
    QString m_sourceAsset;
    QString m_inputFolder;
    QString m_outputFolderContainer;
    QString m_outputFolder;
    bool m_showAdvancedOptions = false;
    QString m_conversionGuid;
    QString m_message;
    uint64_t m_startConversionTime;
    uint64_t m_endConversionTime;
    ConversionOptions m_options;
    uint64_t m_availableOptions = (uint64_t)ConversionOption::All;

    QString GetPlaceholderName() const;
    QString GetPlaceholderInputFolder() const;
};

void GetSrcAssetAxisMapping(const QString& file, Axis& out1, Axis& out2, Axis& out3);
uint64_t GetAssetConversionOptions(const QString& file);