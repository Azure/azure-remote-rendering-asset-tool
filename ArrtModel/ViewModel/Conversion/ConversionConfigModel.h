#pragma once

#include <Model/IncludesAzureStorage.h>
#include <QJsonDocument>
#include <QObject>

namespace ArrtConversion
{
    // to be able to reflect enums used in a non QObject (Config struct) we need to reflect them in a Q_NAMESPACE.

    Q_NAMESPACE

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

    Q_ENUM_NS(Sideness);
    Q_ENUM_NS(Axis);
    Q_ENUM_NS(SceneGraphMode);
    Q_ENUM_NS(VertexPosition);
    Q_ENUM_NS(VertexColor);
    Q_ENUM_NS(VertexVector);
    Q_ENUM_NS(VertexTextureCoord);


    // struct holding the configuration values for an conversion. It reflects
    // https://dev.azure.com/arrClient/_git/arrClient?path=%2FDocumentation%2Farticles%2FconfiguringConversion.html
    // <TODO> once the documentation is distributed or exposed in github, or via docs.microsoft.com, change this comment
    struct Config
    {
        bool operator==(const Config& c) const;
        float m_scaling = 1.0f;
        bool m_recenterToOrigin = false;
        Sideness m_opaqueMaterialDefaultSidedness = Sideness::DoubleSided;
        QString m_material_override;
        bool m_gammaToLinearMaterial = false;
        bool m_gammaToLinearVertex = false;
        SceneGraphMode m_sceneGraphMode = SceneGraphMode::Dynamic;
        bool m_generateCollisionMesh = true;
        bool m_unlitMaterials = false;
        bool m_fbxAssumeMetallic = true;
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
        VertexTextureCoord m_vertexTexCoord1 = VertexTextureCoord::None;
    };
} // namespace ArrtConversion


class ParameterModel;
class AzureStorageManager;

// model used by the view with the conversion configuration (right before starting the conversion). It exposes the models for the controls for each property

class ConversionConfigModel : public QObject, protected ArrtConversion::Config
{
    Q_OBJECT

    // Qt reflected properties, wrapping each field in Config
    Q_PROPERTY(float scaling MEMBER m_scaling);
    Q_PROPERTY(bool recenterToOrigin MEMBER m_recenterToOrigin);
    Q_PROPERTY(ArrtConversion::Sideness opaqueMaterialDefaultSidedness MEMBER m_opaqueMaterialDefaultSidedness);
    Q_PROPERTY(QString material_override MEMBER m_material_override);
    Q_PROPERTY(bool gammaToLinearMaterial MEMBER m_gammaToLinearMaterial);
    Q_PROPERTY(bool gammaToLinearVertex MEMBER m_gammaToLinearVertex);
    Q_PROPERTY(ArrtConversion::SceneGraphMode sceneGraphMode MEMBER m_sceneGraphMode);
    Q_PROPERTY(bool generateCollisionMesh MEMBER m_generateCollisionMesh);
    Q_PROPERTY(bool unlitMaterials MEMBER m_unlitMaterials);
    Q_PROPERTY(bool fbxAssumeMetallic MEMBER m_fbxAssumeMetallic);
    Q_PROPERTY(ArrtConversion::Axis axis1 MEMBER m_axis1);
    Q_PROPERTY(ArrtConversion::Axis axis2 MEMBER m_axis2);
    Q_PROPERTY(ArrtConversion::Axis axis3 MEMBER m_axis3);
    Q_PROPERTY(ArrtConversion::VertexPosition vertexPosition MEMBER m_vertexPosition);
    Q_PROPERTY(ArrtConversion::VertexColor vertexColor0 MEMBER m_vertexColor0);
    Q_PROPERTY(ArrtConversion::VertexColor vertexColor1 MEMBER m_vertexColor1);
    Q_PROPERTY(ArrtConversion::VertexVector vertexNormal MEMBER m_vertexNormal);
    Q_PROPERTY(ArrtConversion::VertexVector vertexTangent MEMBER m_vertexTangent);
    Q_PROPERTY(ArrtConversion::VertexVector vertexBinormal MEMBER m_vertexBinormal);
    Q_PROPERTY(ArrtConversion::VertexTextureCoord vertexTexCoord0 MEMBER m_vertexTexCoord0);
    Q_PROPERTY(ArrtConversion::VertexTextureCoord vertexTexCoord1 MEMBER m_vertexTexCoord1);

public:
    ConversionConfigModel(AzureStorageManager* storageManager, QObject* parent = {});

    bool isDefault() const;
    void restoreToDefault();

    // resets the config to an unloaded/default state
    void unload();
    // load a configuration in the folder/container
    void load(utility::string_t folder, azure::storage::storage_uri container);
    // save the last loaded configuration
    bool save();

    const QList<ParameterModel*>& getControls() const { return m_controls; }

private:
    AzureStorageManager* const m_storageManager;
    QList<ParameterModel*> m_controls;

    utility::string_t m_folder;
    azure::storage::storage_uri m_container;

    // create a Config instance from a Json document
    static ArrtConversion::Config initFromJson(const QJsonDocument& json);
    // create a Json document from a Config instance
    static QJsonDocument createJson(const ArrtConversion::Config& config);
};
