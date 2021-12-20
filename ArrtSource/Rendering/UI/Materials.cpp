// for weird reasons, if this isn't included before App/AppWindow.h in this file, there is a compiler error (probably preprocessor polution)
#include <Rendering/IncludeAzureRemoteRendering.h>

#include <App/AppWindow.h>
#include <Rendering/UI/ScenegraphModel.h>

// Color pickers return colors in "Gamma space" (sRGB). Just like photos and everything that's not just a mathematical construct.
// ARR takes colors in "linear space", so all color values must be converted from sRGB/Gamma space to linear space,
// otherwise the chosen color want match the color that you see on screen.
static float GammaToLinear(float gamma)
{
    return gamma <= 0.04045f ? (gamma / 12.92f) : (powf((gamma + 0.055f) / 1.055f, 2.4f));
}

static float LinearToGamma(float linear)
{
    // assuming we have linear color (not CIE xyY or CIE XYZ)
    return linear <= 0.0031308f ? (12.92f * linear) : (1.055f * powf(linear, 1.0f / 2.4f) - 0.055f);
}

static void GetEntityMaterials(std::map<unsigned long long, RR::ApiHandle<RR::Material>>& allMaterials, const RR::ApiHandle<RR::Entity>& entity)
{
    std::vector<RR::ApiHandle<RR::Material>> materials;
    std::vector<RR::ApiHandle<RR::ComponentBase>> components;
    entity->GetComponents(components);

    for (auto&& component : components)
    {
        if (component->GetType() == RR::ObjectType::MeshComponent)
        {
            const RR::ApiHandle<RR::MeshComponent> meshComponent = component.as<RR::MeshComponent>();

            if (auto mesh = meshComponent->GetMesh())
            {
                mesh->GetMaterials(materials);

                for (auto mat : materials)
                {
                    allMaterials[mat->GetHandle()] = mat;
                }
            }
        }
    }
}

static void GetChildEntities(std::map<unsigned long long, RR::ApiHandle<RR::Entity>>& allEntities, const RR::ApiHandle<RR::Entity>& entity)
{
    if (allEntities.find(entity->GetHandle()) != allEntities.end())
        return;

    allEntities[entity->GetHandle()] = entity;

    std::vector<RR::ApiHandle<RR::Entity>> children;
    entity->GetChildren(children);

    for (const auto& child : children)
    {
        GetChildEntities(allEntities, child);
    }
}

void ArrtAppWindow::UpdateMaterialsList()
{
    std::map<unsigned long long, RR::ApiHandle<RR::Entity>> allEntities;
    std::map<unsigned long long, RR::ApiHandle<RR::Material>> allMaterials;

    for (auto index : ScenegraphView->selectionModel()->selectedIndexes())
    {
        auto entity = m_scenegraphModel->GetEntityHandle(index);
        GetChildEntities(allEntities, entity);
    }

    for (const auto& entity : allEntities)
    {
        GetEntityMaterials(allMaterials, entity.second);
    }

    // no change -> early out
    if (allMaterials == m_allMaterialsPreviously)
        return;

    m_allMaterialsPreviously = allMaterials;

    MaterialsList->clear();
    m_materialsList.clear();

    if (!allMaterials.empty())
    {
        // update the list of materials that is currently displayed

        m_materialsList.reserve(allMaterials.size());

        for (const auto& material : allMaterials)
        {
            m_materialsList.push_back(material.second);
        }

        for (const auto& material : allMaterials)
        {
            std::string matName;
            material.second->GetName(matName);

            MaterialsList->addItem(matName.c_str());
        }

        MaterialsList->setCurrentRow(0);
    }
}

static QString GetTextureName(const RR::ApiHandle<RR::Texture>& tex)
{
    if (!tex)
    {
        return {};
    }

    std::string name;
    tex->GetName(name);

    QString res = name.c_str();
    int firstSlash = res.indexOf("/");

    if (firstSlash >= 0)
    {
        res = res.mid(firstSlash + 1);
    }

    return res;
}

void ArrtAppWindow::on_MaterialsList_itemSelectionChanged()
{
    m_selectedMaterial = -1;

    const auto& selection = MaterialsList->selectionModel()->selectedIndexes();

    if (!selection.isEmpty())
    {
        m_selectedMaterial = selection[0].row();
    }

    SetMaterialUI();
}


void ArrtAppWindow::ShowMaterialUI()
{
    bool matPbr = false;
    bool matCol = false;

    if (m_selectedMaterial >= 0)
    {
        const auto& materialBase = m_materialsList[m_selectedMaterial];

        if (materialBase.valid() && materialBase->GetValid())
        {
            matPbr = materialBase->GetMaterialSubType() == RR::MaterialType::Pbr;
            matCol = materialBase->GetMaterialSubType() == RR::MaterialType::Color;
        }
    }

    const bool matAny = matPbr || matCol;

    // Both Material Types
    VertexColorCheck->setVisible(matAny);
    DoubleSidedCheck->setVisible(matAny);
    FadeCheck->setVisible(matAny);
    AlphaClipCheck->setVisible(matAny);
    WriteDepthCheck->setVisible(matAny);
    FresnelCheck->setVisible(matAny);
    AlbedoColorLabel->setVisible(matAny);
    AlbedoColorPicker->setVisible(matAny);
    AlbedoTextureLabel->setVisible(matAny);
    AlbedoTexture->setVisible(matAny);
    TexScaleLabel->setVisible(matAny);
    TexOffsetLabel->setVisible(matAny);
    TexScaleXLabel->setVisible(matAny);
    TexScaleYLabel->setVisible(matAny);
    TexOffsetXLabel->setVisible(matAny);
    TexOffsetYLabel->setVisible(matAny);
    TextureScaleX->setVisible(matAny);
    TextureScaleY->setVisible(matAny);
    TextureOffsetX->setVisible(matAny);
    TextureOffsetY->setVisible(matAny);
    FadeOutLabel->setVisible(matAny);
    FadeOutSpinner->setVisible(matAny);
    AlphaClipLabel->setVisible(matAny);
    AlphaThresholdSpinner->setVisible(matAny);
    FresnelColorLabel->setVisible(matAny);
    FresnelExpLabel->setVisible(matAny);
    FresnelColorPicker->setVisible(matAny);
    FresnelExponentSpinner->setVisible(matAny);

    // Color Material Only
    TransparencyModeLabel->setVisible(matCol);
    TransparencyModeCombo->setVisible(matCol);
    VertexMixLabel->setVisible(matCol);
    VertexMixSpinner->setVisible(matCol);

    // PBR Material Only
    NormalMap->setVisible(matPbr);
    AoMap->setVisible(matPbr);
    RoughnessMap->setVisible(matPbr);
    MetalnessMap->setVisible(matPbr);
    RoughnessSpinner->setVisible(matPbr);
    MetalnessSpinner->setVisible(matPbr);
    AoScaleSpinner->setVisible(matPbr);
    VertexAlphaModeCombo->setVisible(matPbr);
    TransparentCheck->setVisible(matPbr);
    SpecularCheck->setVisible(matPbr);
    VertexModeLabel->setVisible(matPbr);
    NormalMapLabel->setVisible(matPbr);
    RoughnessValueLabel->setVisible(matPbr);
    RoughnessMapLabel->setVisible(matPbr);
    MetalnessValueLabel->setVisible(matPbr);
    MetalnessMapLabel->setVisible(matPbr);
    AoScaleLabel->setVisible(matPbr);
    AoMapLabel->setVisible(matPbr);
}

void ArrtAppWindow::SetMaterialUI()
{
    ShowMaterialUI();

    if (m_selectedMaterial < 0)
        return;

    const auto& materialBase = m_materialsList[m_selectedMaterial];

    if (!materialBase.valid() || !materialBase->GetValid())
        return;

    if (materialBase->GetMaterialSubType() == RR::MaterialType::Color)
    {
        if (auto material = materialBase.as<RR::ColorMaterial>())
        {
            TextureScaleX->setValue(material->GetTexCoordScale().X);
            TextureScaleY->setValue(material->GetTexCoordScale().Y);

            TextureOffsetX->setValue(material->GetTexCoordOffset().X);
            TextureOffsetY->setValue(material->GetTexCoordOffset().Y);

            AlbedoTexture->setText(GetTextureName(material->GetAlbedoTexture()));

            {
                auto ac = material->GetAlbedoColor();
                ac.R = LinearToGamma(ac.R);
                ac.G = LinearToGamma(ac.G);
                ac.B = LinearToGamma(ac.B);
                AlbedoColorPicker->SetColor(QColor::fromRgbF(ac.R, ac.G, ac.B, ac.A), false);
            }

            AlphaThresholdSpinner->setValue(material->GetAlphaClipThreshold());
            FadeOutSpinner->setValue(material->GetFadeOut());

            {
                auto fc = material->GetFresnelEffectColor();
                fc.R = LinearToGamma(fc.R);
                fc.G = LinearToGamma(fc.G);
                fc.B = LinearToGamma(fc.B);
                FresnelColorPicker->SetColor(QColor::fromRgbF(fc.R, fc.G, fc.B, fc.A), false);
            }

            FresnelExponentSpinner->setValue(material->GetFresnelEffectExponent());

            const uint32_t flags = (uint32_t)material->GetColorFlags();

            VertexColorCheck->setChecked(flags & (uint32_t)RR::ColorMaterialFeatures::UseVertexColor);
            DoubleSidedCheck->setChecked(flags & (uint32_t)RR::ColorMaterialFeatures::DoubleSided);
            AlphaClipCheck->setChecked(flags & (uint32_t)RR::ColorMaterialFeatures::AlphaClipped);
            FadeCheck->setChecked(flags & (uint32_t)RR::ColorMaterialFeatures::FadeToBlack);
            FresnelCheck->setChecked(flags & (uint32_t)RR::ColorMaterialFeatures::FresnelEffect);
            WriteDepthCheck->setChecked(flags & (uint32_t)RR::ColorMaterialFeatures::TransparencyWritesDepth);


            TransparencyModeCombo->setCurrentIndex((int)material->GetColorTransparencyMode());
            VertexMixSpinner->setValue(material->GetVertexMix());
        }
    }

    if (materialBase->GetMaterialSubType() == RR::MaterialType::Pbr)
    {
        if (auto material = materialBase.as<RR::PbrMaterial>())
        {
            TextureScaleX->setValue(material->GetTexCoordScale().X);
            TextureScaleY->setValue(material->GetTexCoordScale().Y);

            TextureOffsetX->setValue(material->GetTexCoordOffset().X);
            TextureOffsetY->setValue(material->GetTexCoordOffset().Y);

            AlbedoTexture->setText(GetTextureName(material->GetAlbedoTexture()));
            NormalMap->setText(GetTextureName(material->GetNormalMap()));
            AoMap->setText(GetTextureName(material->GetAOMap()));
            RoughnessMap->setText(GetTextureName(material->GetRoughnessMap()));
            MetalnessMap->setText(GetTextureName(material->GetMetalnessMap()));

            {
                auto ac = material->GetAlbedoColor();
                ac.R = LinearToGamma(ac.R);
                ac.G = LinearToGamma(ac.G);
                ac.B = LinearToGamma(ac.B);
                AlbedoColorPicker->SetColor(QColor::fromRgbF(ac.R, ac.G, ac.B, ac.A), false);
            }

            RoughnessSpinner->setValue(material->GetRoughness());
            MetalnessSpinner->setValue(material->GetMetalness());
            AoScaleSpinner->setValue(material->GetAOScale());
            AlphaThresholdSpinner->setValue(material->GetAlphaClipThreshold());
            FadeOutSpinner->setValue(material->GetFadeOut());

            {
                auto fc = material->GetFresnelEffectColor();
                fc.R = LinearToGamma(fc.R);
                fc.G = LinearToGamma(fc.G);
                fc.B = LinearToGamma(fc.B);
                FresnelColorPicker->SetColor(QColor::fromRgbF(fc.R, fc.G, fc.B, fc.A), false);
            }

            FresnelExponentSpinner->setValue(material->GetFresnelEffectExponent());
            VertexAlphaModeCombo->setCurrentIndex((int)material->GetPbrVertexAlphaMode());

            const uint32_t flags = (uint32_t)material->GetPbrFlags();

            TransparentCheck->setChecked(flags & (uint32_t)RR::PbrMaterialFeatures::TransparentMaterial);
            VertexColorCheck->setChecked(flags & (uint32_t)RR::PbrMaterialFeatures::UseVertexColor);
            DoubleSidedCheck->setChecked(flags & (uint32_t)RR::PbrMaterialFeatures::DoubleSided);
            SpecularCheck->setChecked(flags & (uint32_t)RR::PbrMaterialFeatures::SpecularHighlights);
            AlphaClipCheck->setChecked(flags & (uint32_t)RR::PbrMaterialFeatures::AlphaClipped);
            FadeCheck->setChecked(flags & (uint32_t)RR::PbrMaterialFeatures::FadeToBlack);
            FresnelCheck->setChecked(flags & (uint32_t)RR::PbrMaterialFeatures::FresnelEffect);
            WriteDepthCheck->setChecked(flags & (uint32_t)RR::PbrMaterialFeatures::TransparencyWritesDepth);
        }
    }
}

void ArrtAppWindow::on_AlbedoColorPicker_ColorChanged(const QColor& newColor)
{
    if (m_selectedMaterial < 0)
        return;

    RR::Color4 c;
    c.R = GammaToLinear(newColor.redF());
    c.G = GammaToLinear(newColor.greenF());
    c.B = GammaToLinear(newColor.blueF());
    c.A = newColor.alphaF();

    if (m_materialsList[m_selectedMaterial]->GetMaterialSubType() == RR::MaterialType::Pbr)
    {
        auto mat = m_materialsList[m_selectedMaterial].as<RR::PbrMaterial>();
        mat->SetAlbedoColor(c);
    }
    else
    {
        auto mat = m_materialsList[m_selectedMaterial].as<RR::ColorMaterial>();
        mat->SetAlbedoColor(c);
    }
}

void ArrtAppWindow::on_TransparentCheck_stateChanged(int state)
{
    if (m_selectedMaterial < 0)
        return;

    if (m_materialsList[m_selectedMaterial]->GetMaterialSubType() == RR::MaterialType::Pbr)
    {
        auto mat = m_materialsList[m_selectedMaterial].as<RR::PbrMaterial>();
        uint32_t flags = (uint32_t)mat->GetPbrFlags();

        if (state == Qt::Checked)
            flags |= (uint32_t)RR::PbrMaterialFeatures::TransparentMaterial;
        else
            flags &= ~(uint32_t)RR::PbrMaterialFeatures::TransparentMaterial;

        mat->SetPbrFlags((RR::PbrMaterialFeatures)flags);
    }
}

void ArrtAppWindow::on_WriteDepthCheck_stateChanged(int state)
{
    if (m_selectedMaterial < 0)
        return;

    if (m_materialsList[m_selectedMaterial]->GetMaterialSubType() == RR::MaterialType::Pbr)
    {
        auto mat = m_materialsList[m_selectedMaterial].as<RR::PbrMaterial>();
        uint32_t flags = (uint32_t)mat->GetPbrFlags();

        if (state == Qt::Checked)
            flags |= (uint32_t)RR::PbrMaterialFeatures::TransparencyWritesDepth;
        else
            flags &= ~(uint32_t)RR::PbrMaterialFeatures::TransparencyWritesDepth;

        mat->SetPbrFlags((RR::PbrMaterialFeatures)flags);
    }
    else
    {
        auto mat = m_materialsList[m_selectedMaterial].as<RR::ColorMaterial>();
        uint32_t flags = (uint32_t)mat->GetColorFlags();

        if (state == Qt::Checked)
            flags |= (uint32_t)RR::ColorMaterialFeatures::TransparencyWritesDepth;
        else
            flags &= ~(uint32_t)RR::ColorMaterialFeatures::TransparencyWritesDepth;

        mat->SetColorFlags((RR::ColorMaterialFeatures)flags);
    }
}

void ArrtAppWindow::on_TransparencyModeCombo_currentIndexChanged(int index)
{
    if (m_selectedMaterial < 0)
        return;

    if (m_materialsList[m_selectedMaterial]->GetMaterialSubType() == RR::MaterialType::Color)
    {
        auto mat = m_materialsList[m_selectedMaterial].as<RR::ColorMaterial>();
        mat->SetColorTransparencyMode((RR::ColorTransparencyMode)index);
    }
}

void ArrtAppWindow::on_AlphaClipCheck_stateChanged(int state)
{
    if (m_selectedMaterial < 0)
        return;

    if (m_materialsList[m_selectedMaterial]->GetMaterialSubType() == RR::MaterialType::Pbr)
    {
        auto mat = m_materialsList[m_selectedMaterial].as<RR::PbrMaterial>();
        uint32_t flags = (uint32_t)mat->GetPbrFlags();

        if (state == Qt::Checked)
            flags |= (uint32_t)RR::PbrMaterialFeatures::AlphaClipped;
        else
            flags &= ~(uint32_t)RR::PbrMaterialFeatures::AlphaClipped;

        mat->SetPbrFlags((RR::PbrMaterialFeatures)flags);
    }
    else
    {
        auto mat = m_materialsList[m_selectedMaterial].as<RR::ColorMaterial>();
        uint32_t flags = (uint32_t)mat->GetColorFlags();

        if (state == Qt::Checked)
            flags |= (uint32_t)RR::ColorMaterialFeatures::AlphaClipped;
        else
            flags &= ~(uint32_t)RR::ColorMaterialFeatures::AlphaClipped;

        mat->SetColorFlags((RR::ColorMaterialFeatures)flags);
    }
}

void ArrtAppWindow::on_AlphaThresholdSpinner_valueChanged(double d)
{
    if (m_selectedMaterial < 0)
        return;

    if (m_materialsList[m_selectedMaterial]->GetMaterialSubType() == RR::MaterialType::Pbr)
    {
        auto mat = m_materialsList[m_selectedMaterial].as<RR::PbrMaterial>();
        mat->SetAlphaClipThreshold((float)d);
    }
    else
    {
        auto mat = m_materialsList[m_selectedMaterial].as<RR::ColorMaterial>();
        mat->SetAlphaClipThreshold((float)d);
    }
}

void ArrtAppWindow::on_DoubleSidedCheck_stateChanged(int state)
{
    if (m_selectedMaterial < 0)
        return;

    if (m_materialsList[m_selectedMaterial]->GetMaterialSubType() == RR::MaterialType::Pbr)
    {
        auto mat = m_materialsList[m_selectedMaterial].as<RR::PbrMaterial>();
        uint32_t flags = (uint32_t)mat->GetPbrFlags();

        if (state == Qt::Checked)
            flags |= (uint32_t)RR::PbrMaterialFeatures::DoubleSided;
        else
            flags &= ~(uint32_t)RR::PbrMaterialFeatures::DoubleSided;

        mat->SetPbrFlags((RR::PbrMaterialFeatures)flags);
    }
    else
    {
        auto mat = m_materialsList[m_selectedMaterial].as<RR::ColorMaterial>();
        uint32_t flags = (uint32_t)mat->GetColorFlags();

        if (state == Qt::Checked)
            flags |= (uint32_t)RR::ColorMaterialFeatures::DoubleSided;
        else
            flags &= ~(uint32_t)RR::ColorMaterialFeatures::DoubleSided;

        mat->SetColorFlags((RR::ColorMaterialFeatures)flags);
    }
}

void ArrtAppWindow::on_SpecularCheck_stateChanged(int state)
{
    if (m_selectedMaterial < 0)
        return;

    if (m_materialsList[m_selectedMaterial]->GetMaterialSubType() == RR::MaterialType::Pbr)
    {
        auto mat = m_materialsList[m_selectedMaterial].as<RR::PbrMaterial>();
        uint32_t flags = (uint32_t)mat->GetPbrFlags();

        if (state == Qt::Checked)
            flags |= (uint32_t)RR::PbrMaterialFeatures::SpecularHighlights;
        else
            flags &= ~(uint32_t)RR::PbrMaterialFeatures::SpecularHighlights;

        mat->SetPbrFlags((RR::PbrMaterialFeatures)flags);
    }
}

void ArrtAppWindow::on_VertexColorCheck_stateChanged(int state)
{
    if (m_selectedMaterial < 0)
        return;

    if (m_materialsList[m_selectedMaterial]->GetMaterialSubType() == RR::MaterialType::Pbr)
    {
        auto mat = m_materialsList[m_selectedMaterial].as<RR::PbrMaterial>();
        uint32_t flags = (uint32_t)mat->GetPbrFlags();

        if (state == Qt::Checked)
            flags |= (uint32_t)RR::PbrMaterialFeatures::UseVertexColor;
        else
            flags &= ~(uint32_t)RR::PbrMaterialFeatures::UseVertexColor;

        mat->SetPbrFlags((RR::PbrMaterialFeatures)flags);
    }
    else
    {
        auto mat = m_materialsList[m_selectedMaterial].as<RR::ColorMaterial>();
        uint32_t flags = (uint32_t)mat->GetColorFlags();

        if (state == Qt::Checked)
            flags |= (uint32_t)RR::ColorMaterialFeatures::UseVertexColor;
        else
            flags &= ~(uint32_t)RR::ColorMaterialFeatures::UseVertexColor;

        mat->SetColorFlags((RR::ColorMaterialFeatures)flags);
    }
}

void ArrtAppWindow::on_VertexMixSpinner_valueChanged(double d)
{
    if (m_selectedMaterial < 0)
        return;

    if (m_materialsList[m_selectedMaterial]->GetMaterialSubType() == RR::MaterialType::Color)
    {
        auto mat = m_materialsList[m_selectedMaterial].as<RR::ColorMaterial>();
        mat->SetVertexMix((float)d);
    }
}

void ArrtAppWindow::on_VertexAlphaModeCombo_currentIndexChanged(int index)
{
    if (m_selectedMaterial < 0)
        return;

    if (m_materialsList[m_selectedMaterial]->GetMaterialSubType() == RR::MaterialType::Pbr)
    {
        auto mat = m_materialsList[m_selectedMaterial].as<RR::PbrMaterial>();
        mat->SetPbrVertexAlphaMode((RR::PbrVertexAlphaMode)index);
    }
}

void ArrtAppWindow::on_RoughnessSpinner_valueChanged(double d)
{
    if (m_selectedMaterial < 0)
        return;

    if (m_materialsList[m_selectedMaterial]->GetMaterialSubType() == RR::MaterialType::Pbr)
    {
        auto mat = m_materialsList[m_selectedMaterial].as<RR::PbrMaterial>();
        mat->SetRoughness((float)d);
    }
}

void ArrtAppWindow::on_MetalnessSpinner_valueChanged(double d)
{
    if (m_selectedMaterial < 0)
        return;

    if (m_materialsList[m_selectedMaterial]->GetMaterialSubType() == RR::MaterialType::Pbr)
    {
        auto mat = m_materialsList[m_selectedMaterial].as<RR::PbrMaterial>();
        mat->SetMetalness((float)d);
    }
}

void ArrtAppWindow::on_AoScaleSpinner_valueChanged(double d)
{
    if (m_selectedMaterial < 0)
        return;

    if (m_materialsList[m_selectedMaterial]->GetMaterialSubType() == RR::MaterialType::Pbr)
    {
        auto mat = m_materialsList[m_selectedMaterial].as<RR::PbrMaterial>();
        mat->SetAOScale((float)d);
    }
}

void ArrtAppWindow::on_TextureScaleX_valueChanged(double d)
{
    if (m_selectedMaterial < 0)
        return;

    if (m_materialsList[m_selectedMaterial]->GetMaterialSubType() == RR::MaterialType::Pbr)
    {
        auto mat = m_materialsList[m_selectedMaterial].as<RR::PbrMaterial>();
        RR::Float2 val = mat->GetTexCoordScale();
        val.X = (float)d;
        mat->SetTexCoordScale(val);
    }
    else
    {
        auto mat = m_materialsList[m_selectedMaterial].as<RR::ColorMaterial>();
        RR::Float2 val = mat->GetTexCoordScale();
        val.X = (float)d;
        mat->SetTexCoordScale(val);
    }
}

void ArrtAppWindow::on_TextureScaleY_valueChanged(double d)
{
    if (m_selectedMaterial < 0)
        return;

    if (m_materialsList[m_selectedMaterial]->GetMaterialSubType() == RR::MaterialType::Pbr)
    {
        auto mat = m_materialsList[m_selectedMaterial].as<RR::PbrMaterial>();
        RR::Float2 val = mat->GetTexCoordScale();
        val.Y = (float)d;
        mat->SetTexCoordScale(val);
    }
    else
    {
        auto mat = m_materialsList[m_selectedMaterial].as<RR::ColorMaterial>();
        RR::Float2 val = mat->GetTexCoordScale();
        val.Y = (float)d;
        mat->SetTexCoordScale(val);
    }
}

void ArrtAppWindow::on_TextureOffsetX_valueChanged(double d)
{
    if (m_selectedMaterial < 0)
        return;

    if (m_materialsList[m_selectedMaterial]->GetMaterialSubType() == RR::MaterialType::Pbr)
    {
        auto mat = m_materialsList[m_selectedMaterial].as<RR::PbrMaterial>();
        RR::Float2 val = mat->GetTexCoordOffset();
        val.X = (float)d;
        mat->SetTexCoordOffset(val);
    }
    else
    {
        auto mat = m_materialsList[m_selectedMaterial].as<RR::ColorMaterial>();
        RR::Float2 val = mat->GetTexCoordOffset();
        val.X = (float)d;
        mat->SetTexCoordOffset(val);
    }
}

void ArrtAppWindow::on_TextureOffsetY_valueChanged(double d)
{
    if (m_selectedMaterial < 0)
        return;

    if (m_materialsList[m_selectedMaterial]->GetMaterialSubType() == RR::MaterialType::Pbr)
    {
        auto mat = m_materialsList[m_selectedMaterial].as<RR::PbrMaterial>();
        RR::Float2 val = mat->GetTexCoordOffset();
        val.Y = (float)d;
        mat->SetTexCoordOffset(val);
    }
    else
    {
        auto mat = m_materialsList[m_selectedMaterial].as<RR::ColorMaterial>();
        RR::Float2 val = mat->GetTexCoordOffset();
        val.Y = (float)d;
        mat->SetTexCoordOffset(val);
    }
}

void ArrtAppWindow::on_FadeCheck_stateChanged(int state)
{
    if (m_selectedMaterial < 0)
        return;

    if (m_materialsList[m_selectedMaterial]->GetMaterialSubType() == RR::MaterialType::Pbr)
    {
        auto mat = m_materialsList[m_selectedMaterial].as<RR::PbrMaterial>();
        uint32_t flags = (uint32_t)mat->GetPbrFlags();

        if (state == Qt::Checked)
            flags |= (uint32_t)RR::PbrMaterialFeatures::FadeToBlack;
        else
            flags &= ~(uint32_t)RR::PbrMaterialFeatures::FadeToBlack;

        mat->SetPbrFlags((RR::PbrMaterialFeatures)flags);
    }
    else
    {
        auto mat = m_materialsList[m_selectedMaterial].as<RR::ColorMaterial>();
        uint32_t flags = (uint32_t)mat->GetColorFlags();

        if (state == Qt::Checked)
            flags |= (uint32_t)RR::ColorMaterialFeatures::FadeToBlack;
        else
            flags &= ~(uint32_t)RR::ColorMaterialFeatures::FadeToBlack;

        mat->SetColorFlags((RR::ColorMaterialFeatures)flags);
    }
}

void ArrtAppWindow::on_FadeOutSpinner_valueChanged(double d)
{
    if (m_selectedMaterial < 0)
        return;

    if (m_materialsList[m_selectedMaterial]->GetMaterialSubType() == RR::MaterialType::Pbr)
    {
        auto mat = m_materialsList[m_selectedMaterial].as<RR::PbrMaterial>();
        mat->SetFadeOut((float)d);
    }
    else
    {
        auto mat = m_materialsList[m_selectedMaterial].as<RR::ColorMaterial>();
        mat->SetFadeOut((float)d);
    }
}

void ArrtAppWindow::on_FresnelCheck_stateChanged(int state)
{
    if (m_selectedMaterial < 0)
        return;

    if (m_materialsList[m_selectedMaterial]->GetMaterialSubType() == RR::MaterialType::Pbr)
    {
        auto mat = m_materialsList[m_selectedMaterial].as<RR::PbrMaterial>();
        uint32_t flags = (uint32_t)mat->GetPbrFlags();

        if (state == Qt::Checked)
            flags |= (uint32_t)RR::PbrMaterialFeatures::FresnelEffect;
        else
            flags &= ~(uint32_t)RR::PbrMaterialFeatures::FresnelEffect;

        mat->SetPbrFlags((RR::PbrMaterialFeatures)flags);
    }
    else
    {
        auto mat = m_materialsList[m_selectedMaterial].as<RR::ColorMaterial>();
        uint32_t flags = (uint32_t)mat->GetColorFlags();

        if (state == Qt::Checked)
            flags |= (uint32_t)RR::ColorMaterialFeatures::FresnelEffect;
        else
            flags &= ~(uint32_t)RR::ColorMaterialFeatures::FresnelEffect;

        mat->SetColorFlags((RR::ColorMaterialFeatures)flags);
    }
}

void ArrtAppWindow::on_FresnelColorPicker_ColorChanged(const QColor& newColor)
{
    if (m_selectedMaterial < 0)
        return;

    RR::Color4 c;
    c.R = GammaToLinear(newColor.redF());
    c.G = GammaToLinear(newColor.greenF());
    c.B = GammaToLinear(newColor.blueF());
    c.A = newColor.alphaF();

    if (m_materialsList[m_selectedMaterial]->GetMaterialSubType() == RR::MaterialType::Pbr)
    {
        auto mat = m_materialsList[m_selectedMaterial].as<RR::PbrMaterial>();
        mat->SetFresnelEffectColor(c);
    }
    else
    {
        auto mat = m_materialsList[m_selectedMaterial].as<RR::ColorMaterial>();
        mat->SetFresnelEffectColor(c);
    }
}

void ArrtAppWindow::on_FresnelExponentSpinner_valueChanged(double d)
{
    if (m_selectedMaterial < 0)
        return;

    if (m_materialsList[m_selectedMaterial]->GetMaterialSubType() == RR::MaterialType::Pbr)
    {
        auto mat = m_materialsList[m_selectedMaterial].as<RR::PbrMaterial>();
        mat->SetFresnelEffectExponent((float)d);
    }
    else
    {
        auto mat = m_materialsList[m_selectedMaterial].as<RR::ColorMaterial>();
        mat->SetFresnelEffectExponent((float)d);
    }
}
