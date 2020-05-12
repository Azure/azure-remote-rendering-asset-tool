#include <Model/AzureStorageManager.h>
#include <Model/Log/LogHelpers.h>
#include <Utils/JsonUtils.h>
#include <ViewModel/Conversion/ConversionConfigModel.h>
#include <ViewModel/Parameters/CheckBoxModel.h>
#include <ViewModel/Parameters/ComboBoxModel.h>
#include <ViewModel/Parameters/FloatModel.h>
#include <ViewModel/Parameters/IntegerModel.h>
#include <ViewModel/Parameters/TextModel.h>
#include <codecvt>
#include <locale>
#include <string_view>

namespace JsonUtils
{
#define ENUM_LABELS_BEGIN(type) static const QString s_##type##Labels[] =
#define ENUM_LABELS_END(type)                                                                                        \
    ;                                                                                                                \
    template <>                                                                                                      \
    ArrtConversion::type fromString<ArrtConversion::type>(const QString& s, const ArrtConversion::type defaultValue) \
    {                                                                                                                \
        return fromString<ArrtConversion::type>(s, s_##type##Labels, defaultValue);                                  \
    }                                                                                                                \
    template <>                                                                                                      \
    QString toString<ArrtConversion::type>(const ArrtConversion::type& v)                                            \
    {                                                                                                                \
        return toString<ArrtConversion::type>(v, s_##type##Labels);                                                  \
    }


    ENUM_LABELS_BEGIN(SceneGraphMode){
        QString("none"),
        QString("static"),
        QString("dynamic")} ENUM_LABELS_END(SceneGraphMode)

        ENUM_LABELS_BEGIN(Axis){
            QString("Default"),
            QString("+x"),
            QString("-x"),
            QString("+y"),
            QString("-y"),
            QString("+z"),
            QString("-z")} ENUM_LABELS_END(Axis)

            ENUM_LABELS_BEGIN(VertexPosition){
                QString("32_32_32_FLOAT"),
                QString("16_16_16_16_FLOAT")} ENUM_LABELS_END(VertexPosition)

                ENUM_LABELS_BEGIN(VertexColor){
                    QString("NONE"),
                    QString("8_8_8_8_UNSIGNED_NORMALIZED")} ENUM_LABELS_END(VertexColor);

    ENUM_LABELS_BEGIN(VertexVector){
        QString("NONE"),
        QString("8_8_8_8_SIGNED_NORMALIZED"),
        QString("16_16_16_16_FLOAT")} ENUM_LABELS_END(VertexVector)

        ENUM_LABELS_BEGIN(VertexTextureCoord){
            QString("NONE"),
            QString("32_32_FLOAT"),
            QString("16_16_FLOAT")} ENUM_LABELS_END(VertexTextureCoord)
} // namespace JsonUtils

using namespace JsonUtils;

namespace
{
    const wchar_t s_configFile[] = L"ConversionSettings.json";
}

bool ArrtConversion::Config::operator==(const Config& c) const
{
    return m_scaling == c.m_scaling && m_recenterToOrigin == c.m_recenterToOrigin && m_opaqueMaterialDefaultSidedness == c.m_opaqueMaterialDefaultSidedness && m_material_override == c.m_material_override && m_gammaToLinearMaterial == c.m_gammaToLinearMaterial && m_gammaToLinearVertex == c.m_gammaToLinearVertex && m_sceneGraphMode == c.m_sceneGraphMode && m_generateCollisionMesh == c.m_generateCollisionMesh && m_unlitMaterials == c.m_unlitMaterials && m_fbxAssumeMetallic == c.m_fbxAssumeMetallic && m_axis1 == c.m_axis1 && m_axis2 == c.m_axis2 && m_axis3 == c.m_axis3 && m_vertexPosition == c.m_vertexPosition && m_vertexColor0 == c.m_vertexColor0 && m_vertexColor1 == c.m_vertexColor1 && m_vertexNormal == c.m_vertexNormal && m_vertexTangent == c.m_vertexTangent && m_vertexBinormal == c.m_vertexBinormal && m_vertexTexCoord0 == c.m_vertexTexCoord0 && m_vertexTexCoord1 == c.m_vertexTexCoord1;
}

bool ConversionConfigModel::isDefault() const
{
    const ArrtConversion::Config& thisAsConfig = *this;
    return !(thisAsConfig == ArrtConversion::Config());
}

void ConversionConfigModel::restoreToDefault()
{
    ArrtConversion::Config& thisAsConfig = *this;
    // assign only the config part, to a default constructed one;
    thisAsConfig = {};
}

void ConversionConfigModel::unload()
{
    m_folder = {};
    m_container = {};
    restoreToDefault();
}

void ConversionConfigModel::load(utility::string_t folder, azure::storage::storage_uri container)
{
    m_folder = std::move(folder);
    m_container = std::move(container);

    //check if there is already a config file and reads it

    ArrtConversion::Config& thisAsConfig = *this;
    utility::string_t jsonFile;
    utility::string_t fileLocation = m_folder + s_configFile;

    if (m_storageManager->readTextFileSync(m_container, fileLocation, jsonFile))
    {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        thisAsConfig = initFromJson(QJsonDocument::fromJson(converter.to_bytes(jsonFile).c_str()));

        qInfo(LoggingCategory::conversion)
            << tr("Loading conversion configuration from") << QString::fromStdWString(fileLocation);
    }
    else
    {
        //reset to default
        thisAsConfig = {};
    }
}

bool ConversionConfigModel::save()
{
    if (!m_container.primary_uri().is_empty())
    {
        auto configFileLocation = m_folder + s_configFile;
        QJsonDocument configuration = createJson(*this);
        QString jsonStr = QString::fromUtf8(configuration.toJson());
        qInfo(LoggingCategory::conversion)
            << tr("Uploading conversion configuration to blob container:\n")
            << PrettyJson(configuration.object());
        // it captures m_onAccepted, instead of "this" because this model is usually deleted before the callback completes
        if (m_storageManager->writeTextFileSync(
                jsonStr.toStdWString(), m_container, configFileLocation))
        {
            qInfo(LoggingCategory::conversion)
                << tr("Conversion configuration uploaded to blob container to") << QString::fromStdWString(configFileLocation);
            return true;
        }
        else
        {
            auto location = QString::fromStdWString(m_container.path()) + "/" + QString::fromStdWString(m_folder) + QString::fromStdWString(s_configFile);
            qWarning(LoggingCategory::conversion)
                << tr("Failed to upload configuration to blob container in location ") << location;
        }
    }
    return false;
}


ConversionConfigModel::ConversionConfigModel(AzureStorageManager* storageManager, QObject* parent)
    : QObject(parent)
    , m_storageManager(storageManager)
{
    using namespace std::literals;

    m_controls.push_back(new FloatModel(tr("Scaling"), this, "scaling"sv));
    m_controls.push_back(new CheckBoxModel(tr("Recenter to Origin"), this, "recenterToOrigin"sv));
    m_controls.push_back(new CheckBoxModel(tr("Opaque Material Default Sideness"), this, "opaqueMaterialDefaultSidedness"sv));
    m_controls.push_back(new TextModel(tr("Material Override"), this, "material_override"sv));
    m_controls.push_back(new CheckBoxModel(tr("Gamma to Linear Material"), this, "gammaToLinearMaterial"sv));
    m_controls.push_back(new CheckBoxModel(tr("Gamma to Linear Vertex"), this, "gammaToLinearVertex"sv));
    m_controls.push_back(new ComboBoxModel(tr("Scene Graph Mode"), this, "sceneGraphMode"sv));

    m_controls.push_back(new CheckBoxModel(tr("Generate Collision Mesh"), this, "generateCollisionMesh"sv));
    m_controls.push_back(new CheckBoxModel(tr("Unlit Materials"), this, "unlitMaterials"sv));
    m_controls.push_back(new CheckBoxModel(tr("FBX Assume Metallic"), this, "fbxAssumeMetallic"sv));
    m_controls.push_back(new ComboBoxModel(tr("Axis [0]"), this, "axis1"sv));
    m_controls.push_back(new ComboBoxModel(tr("Axis [1]"), this, "axis2"sv));
    m_controls.push_back(new ComboBoxModel(tr("Axis [2]"), this, "axis3"sv));

    m_controls.push_back(new ComboBoxModel(tr("Vertex Position"), this, "vertexPosition"));
    m_controls.push_back(new ComboBoxModel(tr("Vertex Color 0"), this, "vertexColor0"sv));
    m_controls.push_back(new ComboBoxModel(tr("Vertex Color 1"), this, "vertexColor1"sv));
    m_controls.push_back(new ComboBoxModel(tr("Vertex Normal"), this, "vertexNormal"sv));
    m_controls.push_back(new ComboBoxModel(tr("Vertex Tangent"), this, "vertexTangent"sv));
    m_controls.push_back(new ComboBoxModel(tr("Vertex Binormal"), this, "vertexBinormal"sv));
    m_controls.push_back(new ComboBoxModel(tr("Vertex TextureCoord0"), this, "vertexTexCoord0"sv));
    m_controls.push_back(new ComboBoxModel(tr("Vertex TextureCoord1"), this, "vertexTexCoord1"sv));
}

ArrtConversion::Config ConversionConfigModel::initFromJson(const QJsonDocument& json)
{
    ArrtConversion::Config cfg;

    const QJsonObject& root = json.object();

    if (!root.isEmpty())
    {
        cfg.m_scaling = fromJson(root, QLatin1String("scaling"), cfg.m_scaling);
        cfg.m_recenterToOrigin = fromJson(root, QLatin1String("recenterToOrigin"), cfg.m_recenterToOrigin);
        cfg.m_opaqueMaterialDefaultSidedness = fromJson(root, QLatin1String("opaqueMaterialDefaultSidedness"), cfg.m_opaqueMaterialDefaultSidedness);
        cfg.m_material_override = fromJson(root, QLatin1String("material-override"), cfg.m_material_override);
        cfg.m_gammaToLinearMaterial = fromJson(root, QLatin1String("gammaToLinearMaterial"), cfg.m_gammaToLinearMaterial);
        cfg.m_gammaToLinearVertex = fromJson(root, QLatin1String("gammaToLinearVertex"), cfg.m_gammaToLinearVertex);
        cfg.m_sceneGraphMode = fromJson(root, QLatin1String("sceneGraphMode"), cfg.m_sceneGraphMode);
        cfg.m_generateCollisionMesh = fromJson(root, QLatin1String("generateCollisionMesh"), cfg.m_generateCollisionMesh);
        cfg.m_unlitMaterials = fromJson(root, QLatin1String("unlitMaterials"), cfg.m_unlitMaterials);
        cfg.m_fbxAssumeMetallic = fromJson(root, QLatin1String("fbxAssumeMetallic"), cfg.m_fbxAssumeMetallic);
        auto axes = root[QLatin1String("axis")].toArray();
        cfg.m_axis1 = fromJson(axes, 0, cfg.m_axis1);
        cfg.m_axis2 = fromJson(axes, 1, cfg.m_axis2);
        cfg.m_axis3 = fromJson(axes, 2, cfg.m_axis3);
        auto vertex = root[QLatin1String("vertex")].toObject();
        cfg.m_vertexPosition = fromJson(vertex, QLatin1String("position"), cfg.m_vertexPosition);
        cfg.m_vertexColor0 = fromJson(vertex, QLatin1String("color0"), cfg.m_vertexColor0);
        cfg.m_vertexColor1 = fromJson(vertex, QLatin1String("color1"), cfg.m_vertexColor1);
        cfg.m_vertexNormal = fromJson(vertex, QLatin1String("normal"), cfg.m_vertexNormal);
        cfg.m_vertexTangent = fromJson(vertex, QLatin1String("tangent"), cfg.m_vertexTangent);
        cfg.m_vertexBinormal = fromJson(vertex, QLatin1String("binormal"), cfg.m_vertexBinormal);
        cfg.m_vertexTexCoord0 = fromJson(vertex, QLatin1String("texcoord0"), cfg.m_vertexTexCoord0);
        cfg.m_vertexTexCoord1 = fromJson(vertex, QLatin1String("texcoord1"), cfg.m_vertexTexCoord1);
    }
    return cfg;
}

QJsonDocument ConversionConfigModel::createJson(const ArrtConversion::Config& config)
{
    QJsonObject root;
    ArrtConversion::Config def;
    {
        toJson(root, QLatin1String("scaling"), config.m_scaling, def.m_scaling);
        toJson(root, QLatin1String("recenterToOrigin"), config.m_recenterToOrigin, def.m_recenterToOrigin);
        toJson(root, QLatin1String("opaqueMaterialDefaultSidedness"), config.m_opaqueMaterialDefaultSidedness, def.m_opaqueMaterialDefaultSidedness);
        toJson(root, QLatin1String("material-override"), config.m_material_override, def.m_material_override);
        toJson(root, QLatin1String("gammaToLinearMaterial"), config.m_gammaToLinearMaterial, def.m_gammaToLinearMaterial);
        toJson(root, QLatin1String("gammaToLinearVertex"), config.m_gammaToLinearVertex, def.m_gammaToLinearVertex);
        toJson(root, QLatin1String("sceneGraphMode"), config.m_sceneGraphMode, def.m_sceneGraphMode);
        toJson(root, QLatin1String("generateCollisionMesh"), config.m_generateCollisionMesh, def.m_generateCollisionMesh);
        toJson(root, QLatin1String("unlitMaterials"), config.m_unlitMaterials, def.m_unlitMaterials);
        toJson(root, QLatin1String("fbxAssumeMetallic"), config.m_fbxAssumeMetallic, def.m_fbxAssumeMetallic);
        QJsonArray axes;
        {
            bool hasValues = false;
            hasValues |= toJson(axes, 0, config.m_axis1, def.m_axis1, ArrtConversion::Axis::Inherit);
            hasValues |= toJson(axes, 1, config.m_axis2, def.m_axis2, ArrtConversion::Axis::Inherit);
            hasValues |= toJson(axes, 2, config.m_axis3, def.m_axis3, ArrtConversion::Axis::Inherit);
            if (hasValues)
            {
                root[QLatin1String("axis")] = axes;
            }
        }
        QJsonObject vertex;
        {
            bool hasValues = false;
            hasValues |= toJson(vertex, QLatin1String("position"), config.m_vertexPosition, def.m_vertexPosition);
            hasValues |= toJson(vertex, QLatin1String("color0"), config.m_vertexColor0, def.m_vertexColor0);
            hasValues |= toJson(vertex, QLatin1String("color1"), config.m_vertexColor1, def.m_vertexColor1);
            hasValues |= toJson(vertex, QLatin1String("normal"), config.m_vertexNormal, def.m_vertexNormal);
            hasValues |= toJson(vertex, QLatin1String("tangent"), config.m_vertexTangent, def.m_vertexTangent);
            hasValues |= toJson(vertex, QLatin1String("binormal"), config.m_vertexBinormal, def.m_vertexBinormal);
            hasValues |= toJson(vertex, QLatin1String("texcoord0"), config.m_vertexTexCoord0, def.m_vertexTexCoord0);
            hasValues |= toJson(vertex, QLatin1String("texcoord1"), config.m_vertexTexCoord1, def.m_vertexTexCoord1);
            if (hasValues)
            {
                root[QLatin1String("vertex")] = vertex;
            }
        }
    }
    return QJsonDocument(root);
}
