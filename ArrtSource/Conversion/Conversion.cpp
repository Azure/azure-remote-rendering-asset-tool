#include "Conversion.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

static QString ToString(Axis value)
{
    switch (value)
    {
        case Axis::Inherit:
            return "default";
        case Axis::PosX:
            return "+x";
        case Axis::NegX:
            return "-x";
        case Axis::PosY:
            return "+y";
        case Axis::NegY:
            return "-y";
        case Axis::PosZ:
            return "+z";
        case Axis::NegZ:
            return "-z";
    }

    return "";
}

static QString ToString(SceneGraphMode value)
{
    switch (value)
    {
        case SceneGraphMode::None:
            return "none";
        case SceneGraphMode::Static:
            return "static";
        case SceneGraphMode::Dynamic:
            return "dynamic";
    }

    return "";
}

static QString ToString(Sideness value)
{
    switch (value)
    {
        case Sideness::SingleSided:
            return "SingleSided";
        case Sideness::DoubleSided:
            return "DoubleSided";
    }

    return "";
}

static QString ToString(VertexPosition value)
{
    switch (value)
    {
        case VertexPosition::Float32x3:
            return "32_32_32_FLOAT";
        case VertexPosition::Float16x3:
            return "16_16_16_16_FLOAT";
    }

    return "";
}

static QString ToString(VertexColor value)
{
    switch (value)
    {
        case VertexColor::None:
            return "NONE";
        case VertexColor::ByteUx4N:
            return "8_8_8_8_UNSIGNED_NORMALIZED";
    }

    return "";
}

static QString ToString(VertexVector value)
{
    switch (value)
    {
        case VertexVector::None:
            return "NONE";
        case VertexVector::ByteSx4N:
            return "8_8_8_8_SIGNED_NORMALIZED";
        case VertexVector::Float16x4:
            return "16_16_16_16_FLOAT";
    }

    return "";
}

static QString ToString(VertexTextureCoord value)
{
    switch (value)
    {
        case VertexTextureCoord::None:
            return "NONE";
        case VertexTextureCoord::Float32x2:
            return "32_32_FLOAT";
        case VertexTextureCoord::Float16x2:
            return "16_16_FLOAT";
    }

    return "";
}

QString ConversionOptions::ToJSON(uint64_t availableOptions) const
{
    const auto flags = availableOptions;

    QJsonObject root;
    {
        if (flags & (uint64_t)ConversionOption::UniformScaling)
            root["scaling"] = m_scaling;

        if (flags & (uint64_t)ConversionOption::RecenterToOrigin)
            root["recenterToOrigin"] = m_recenterToOrigin;

        if (flags & (uint64_t)ConversionOption::FbxAssumeMetallic)
            root["fbxAssumeMetallic"] = m_fbxAssumeMetallic;

        if (flags & (uint64_t)ConversionOption::GammaToLinearMaterial)
        {
            if (m_materialColorSpace != ColorSpaceMode::FormatDefault)
                root["gammaToLinearMaterial"] = m_materialColorSpace == ColorSpaceMode::GammaSpace;
        }

        if (flags & (uint64_t)ConversionOption::GammaToLinearVertex)
        {
            if (m_vertexColorSpace != ColorSpaceMode::FormatDefault)
                root["gammaToLinearVertex"] = m_vertexColorSpace == ColorSpaceMode::GammaSpace;
        }

        if (flags & (uint64_t)ConversionOption::CollisionMesh)
            root["generateCollisionMesh"] = m_generateCollisionMesh;

        if (flags & (uint64_t)ConversionOption::UnlitMaterials)
            root["unlitMaterials"] = m_unlitMaterials;

        if (flags & (uint64_t)ConversionOption::DeduplicateMaterials)
            root["deduplicateMaterials"] = m_deduplicateMaterials;

        if (flags & (uint64_t)ConversionOption::SceneGraphMode)
            root["sceneGraphMode"] = ToString(m_sceneGraphMode);

        if (flags & (uint64_t)ConversionOption::MaterialDefaultSidedness)
            root["opaqueMaterialDefaultSidedness"] = ToString(m_opaqueMaterialDefaultSidedness);

        if (flags & (uint64_t)ConversionOption::AxisMapping)
        {
            QJsonArray axes;
            axes.append(ToString(m_axis1));
            axes.append(ToString(m_axis2));
            axes.append(ToString(m_axis3));

            root[QLatin1String("axis")] = axes;
        }

        {
            QJsonObject vertex;

            if (flags & (uint64_t)ConversionOption::VertexPositionFormat)
            {
                if (m_vertexPosition != VertexPosition::FormatDefault)
                    vertex["position"] = ToString(m_vertexPosition);
            }

            if (flags & (uint64_t)ConversionOption::VertexColor0Format)
            {
                if (m_vertexColor0 != VertexColor::FormatDefault)
                    vertex["color0"] = ToString(m_vertexColor0);
            }

            if (flags & (uint64_t)ConversionOption::VertexColor1Format)
            {
                if (m_vertexColor1 != VertexColor::FormatDefault)
                    vertex["color1"] = ToString(m_vertexColor1);
            }

            if (flags & (uint64_t)ConversionOption::VertexNormalFormat)
            {
                if (m_vertexNormal != VertexVector::FormatDefault)
                    vertex["normal"] = ToString(m_vertexNormal);
            }

            if (flags & (uint64_t)ConversionOption::VertexTangentFormat)
            {
                if (m_vertexTangent != VertexVector::FormatDefault)
                    vertex["tangent"] = ToString(m_vertexTangent);
            }

            if (flags & (uint64_t)ConversionOption::VertexBinormalFormat)
            {
                if (m_vertexBinormal != VertexVector::FormatDefault)
                    vertex["binormal"] = ToString(m_vertexBinormal);
            }

            if (flags & (uint64_t)ConversionOption::VertexTexCoord0Format)
            {
                if (m_vertexTexCoord0 != VertexTextureCoord::FormatDefault)
                    vertex["texcoord0"] = ToString(m_vertexTexCoord0);
            }

            if (flags & (uint64_t)ConversionOption::VertexTexCoord1Format)
            {
                if (m_vertexTexCoord1 != VertexTextureCoord::FormatDefault)
                    vertex["texcoord1"] = ToString(m_vertexTexCoord1);
            }

            if (!vertex.isEmpty())
                root[QLatin1String("vertex")] = vertex;
        }
    }

    QJsonDocument configuration(root);
    return configuration.toJson(QJsonDocument::Indented);
}

void GetSrcAssetAxisMapping(const QString& file, Axis& out1, Axis& out2, Axis& out3)
{
    out1 = Axis::PosX;
    out2 = Axis::PosY;
    out3 = Axis::PosZ;

    if (file.endsWith(".e57", Qt::CaseInsensitive) ||
        file.endsWith(".ply", Qt::CaseInsensitive) ||
        file.endsWith(".xyz", Qt::CaseInsensitive) ||
        file.endsWith(".las", Qt::CaseInsensitive) ||
        file.endsWith(".laz", Qt::CaseInsensitive))
    {
        out1 = Axis::PosX;
        out2 = Axis::PosZ;
        out3 = Axis::NegY;
    }
}

uint64_t GetAssetConversionOptions(const QString& file)
{
    if (file.endsWith(".e57", Qt::CaseInsensitive) ||
        file.endsWith(".ply", Qt::CaseInsensitive) ||
        file.endsWith(".xyz", Qt::CaseInsensitive) ||
        file.endsWith(".las", Qt::CaseInsensitive) ||
        file.endsWith(".laz", Qt::CaseInsensitive))
    {
        return (uint64_t)ConversionOption::UniformScaling |
               (uint64_t)ConversionOption::RecenterToOrigin |
               (uint64_t)ConversionOption::GammaToLinearVertex |
               (uint64_t)ConversionOption::AxisMapping |
               (uint64_t)ConversionOption::CollisionMesh;
    }

    if (file.endsWith(".fbx", Qt::CaseInsensitive))
    {
        return (uint64_t)ConversionOption::All;
    }

    return (uint64_t)ConversionOption::All & ~(uint64_t)ConversionOption::FbxAssumeMetallic;
}
