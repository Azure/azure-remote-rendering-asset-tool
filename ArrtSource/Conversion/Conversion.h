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
    Float32x3 = 0,
    Float16x3
};

enum class VertexColor
{
    None = 0,
    ByteUx4N
};

enum class VertexVector
{
    None = 0,
    ByteSx4N,
    Float16x4,
};

enum class VertexTextureCoord
{
    None = 0,
    Float32x2,
    Float16x2
};

/// All options describing a conversion
struct ConversionOptions
{
    float m_scaling = 1.0f;
    bool m_recenterToOrigin = false;
    Sideness m_opaqueMaterialDefaultSidedness = Sideness::DoubleSided;
    QString m_materialOverride;
    bool m_gammaToLinearMaterial = false;
    bool m_gammaToLinearVertex = false;
    SceneGraphMode m_sceneGraphMode = SceneGraphMode::Dynamic;
    bool m_generateCollisionMesh = true;
    bool m_unlitMaterials = false;
    bool m_fbxAssumeMetallic = true;
    bool m_deduplicateMaterials = true;
    Axis m_axis1 = Axis::PosX;
    Axis m_axis2 = Axis::PosY;
    Axis m_axis3 = Axis::PosZ;
    VertexPosition m_vertexPosition = VertexPosition::Float32x3;
    VertexColor m_vertexColor0 = VertexColor::ByteUx4N;
    VertexColor m_vertexColor1 = VertexColor::None;
    VertexVector m_vertexNormal = VertexVector::ByteSx4N;
    VertexVector m_vertexTangent = VertexVector::ByteSx4N;
    VertexVector m_vertexBinormal = VertexVector::ByteSx4N;
    VertexTextureCoord m_vertexTexCoord0 = VertexTextureCoord::Float32x2;
    VertexTextureCoord m_vertexTexCoord1 = VertexTextureCoord::Float32x2;

    QString ToJSON() const;
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

    QString GetPlaceholderName() const;
    QString GetPlaceholderInputFolder() const;
};
