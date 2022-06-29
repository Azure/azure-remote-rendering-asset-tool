#include <App/AppWindow.h>
#include <QDir>
#include <Storage/UI/BrowseStorageDlg.h>

void ArrtAppWindow::on_ConversionList_currentRowChanged(int row)
{
    RetrieveConversionOptions();
    m_conversionManager->SetSelectedConversion(row);
}

static QString SecToString(uint32_t sec)
{
    uint32_t hours = (sec / (60 * 60));
    sec -= hours * 60 * 60;

    uint32_t minutes = (sec / 60);
    sec -= minutes * 60;

    return QString("%1:%2:%3").arg(hours, 2, 10, (QChar)'0').arg(minutes, 2, 10, (QChar)'0').arg(sec, 2, 10, (QChar)'0');
}

void ArrtAppWindow::UpdateConversionsList()
{
    const auto& conversions = m_conversionManager->GetConversions();

    if (ConversionTab->ConversionList->count() > conversions.size())
    {
        // an element got removed -> clear the entire list and rebuild it
        ConversionTab->ConversionList->clear();
    }

    if (ConversionTab->ConversionList->count() < conversions.size())
    {
        // an element got added (we always append) -> add the new ones

        for (int i = ConversionTab->ConversionList->count(); i < (int)conversions.size(); ++i)
        {
            const Conversion& conv = conversions[i];

            new QListWidgetItem(conv.m_name, ConversionTab->ConversionList);
        }
    }

    // update the display of all items
    for (int i = 0; i < ConversionTab->ConversionList->count(); ++i)
    {
        const Conversion& conv = conversions[i];
        QListWidgetItem* item = ConversionTab->ConversionList->item(i);


        QString text = conv.m_name;
        switch (conv.m_status)
        {
            case ConversionStatus::New:
            {
                item->setIcon(QIcon(":/ArrtApplication/Icons/conversion.svg"));
                text = "<new conversion>";
                break;
            }
            case ConversionStatus::Running:
            {
                const uint64_t duration = QDateTime::currentSecsSinceEpoch() - conv.m_startConversionTime;
                item->setIcon(QIcon(":/ArrtApplication/Icons/conversion_running.svg"));
                text += QString(" (running) [%1]").arg(SecToString(duration));
                break;
            }
            case ConversionStatus::Finished:
            {
                const uint64_t duration = conv.m_endConversionTime - conv.m_startConversionTime;
                item->setIcon(QIcon(":/ArrtApplication/Icons/conversion_succeeded.svg"));
                text += QString(" (succeeded)");

                if (duration > 0)
                {
                    text += QString(" [%1]").arg(SecToString(duration));
                }

                break;
            }
            case ConversionStatus::Failed:
            {
                const uint64_t duration = conv.m_endConversionTime - conv.m_startConversionTime;
                item->setIcon(QIcon(":/ArrtApplication/Icons/conversion_failed.svg"));
                text += QString(" (failed)");

                if (duration > 0)
                {
                    text += QString(" [%1]").arg(SecToString(duration));
                }

                break;
            }
        }

        item->setText(text);
    }
}

void ArrtAppWindow::on_SelectSourceButton_clicked()
{
    RetrieveConversionOptions();

    BrowseStorageDlg dlg(m_storageAccount.get(), StorageEntry::Type::SrcAsset, m_lastStorageSelectSrcContainer, QString(), this);

    if (dlg.exec() == QDialog::Accepted)
    {
        m_lastStorageSelectSrcContainer = dlg.GetSelectedContainer();
        m_conversionManager->SetConversionSourceAsset(dlg.GetSelectedContainer(), dlg.GetSelectedItem());
    }
}

void ArrtAppWindow::on_SelectInputFolderButton_clicked()
{
    RetrieveConversionOptions();

    const auto& conv = m_conversionManager->GetSelectedConversion();

    BrowseStorageDlg dlg(m_storageAccount.get(), StorageEntry::Type::Folder, conv.m_sourceAssetContainer, conv.GetPlaceholderInputFolder(), this);

    if (dlg.exec() == QDialog::Accepted)
    {
        m_conversionManager->SetConversionInputFolder(dlg.GetSelectedItem());
    }
}

void ArrtAppWindow::on_SelectOutputFolderButton_clicked()
{
    RetrieveConversionOptions();

    BrowseStorageDlg dlg(m_storageAccount.get(), StorageEntry::Type::Folder, m_lastStorageSelectDstContainer, QString(), this);

    if (dlg.exec() == QDialog::Accepted)
    {
        m_lastStorageSelectDstContainer = dlg.GetSelectedContainer();
        m_conversionManager->SetConversionOutputFolder(dlg.GetSelectedContainer(), dlg.GetSelectedItem());
    }
}

void ArrtAppWindow::on_StartConversionButton_clicked()
{
    if (!m_conversionManager->IsEditableSelected())
        return;

    RetrieveConversionOptions();

    m_conversionManager->StartConversion();
}

void ArrtAppWindow::UpdateConversionPane()
{
    const Conversion& conv = m_conversionManager->GetSelectedConversion();

    // whether the user can edit this conversion
    {
        const bool allowEditing = (conv.m_status == ConversionStatus::New);
        ConversionTab->ConversionNameInput->setReadOnly(!allowEditing);
        ConversionTab->SelectSourceButton->setEnabled(allowEditing);
        ConversionTab->SelectOutputFolderButton->setEnabled(allowEditing);
        ConversionTab->SelectInputFolderButton->setEnabled(allowEditing && !conv.m_sourceAsset.isEmpty());
        ConversionTab->scrollAreaWidgetContents->setEnabled(allowEditing);

        // enable or disable the start conversion button depending on whether enough data is set
        ConversionTab->StartConversionButton->setEnabled(allowEditing && !conv.m_sourceAsset.isEmpty() && !conv.m_outputFolderContainer.isEmpty());
        ConversionTab->ResetAdvancedButton->setEnabled(allowEditing);
    }

    // general state
    {
        ConversionTab->ConversionNameInput->setText(conv.m_name);
        ConversionTab->SourceAssetLine->setText(conv.m_sourceAssetContainer + ":" + conv.m_sourceAsset);
        ConversionTab->OutputFolderLine->setText(conv.m_outputFolderContainer + ":" + conv.m_outputFolder);

        ConversionTab->InputFolderLine->setText(conv.m_sourceAssetContainer + ":" + conv.m_inputFolder);

        ConversionTab->ConversionOptionsCheckbox->blockSignals(true);
        ConversionTab->ConversionOptionsCheckbox->setChecked(conv.m_showAdvancedOptions);
        ConversionTab->ConversionOptionsCheckbox->blockSignals(false);
        ConversionTab->ConversionNameInput->setPlaceholderText(conv.GetPlaceholderName());
    }

    switch (conv.m_status)
    {
        case ConversionStatus::New:
            ConversionTab->ConversionMessage->setText("Conversion not started");
            break;
        case ConversionStatus::Finished:
            ConversionTab->ConversionMessage->setText("Conversion finished successfully");
            break;
        case ConversionStatus::Running:
            ConversionTab->ConversionMessage->setText("Conversion currently running");
            break;
        case ConversionStatus::Failed:
            ConversionTab->ConversionMessage->setText(QString("Conversion failed: %1").arg(conv.m_message.isEmpty() ? "(no details)" : conv.m_message));
            break;
    }

    // show advanced options
    {
        if (conv.m_showAdvancedOptions)
        {
            ConversionTab->OptionsSpacer->changeSize(0, 0, QSizePolicy::Ignored, QSizePolicy::Ignored);
        }
        else
        {
            ConversionTab->OptionsSpacer->changeSize(0, 0, QSizePolicy::Ignored, QSizePolicy::Expanding);
        }

        ConversionTab->OptionsArea->setVisible(conv.m_showAdvancedOptions);
    }

    // set advanced options
    {
        const auto& opt = conv.m_options;

        ConversionTab->ScalingSpinbox->setValue(opt.m_scaling);
        ConversionTab->RecenterToOriginCheckbox->setChecked(opt.m_recenterToOrigin);
        ConversionTab->DefaultSidednessCombo->setCurrentIndex((int)opt.m_opaqueMaterialDefaultSidedness);
        ConversionTab->GammaToLinearMaterialCombo->setCurrentIndex((int)opt.m_materialColorSpace);
        ConversionTab->GammaToLinearVertexCombo->setCurrentIndex((int)opt.m_vertexColorSpace);
        ConversionTab->ScenegraphModeCombo->setCurrentIndex((int)opt.m_sceneGraphMode);
        ConversionTab->CollisionMeshCheckbox->setChecked(opt.m_generateCollisionMesh);
        ConversionTab->UnlitMaterialsCheckbox->setChecked(opt.m_unlitMaterials);
        ConversionTab->FbxAssumeMetallicCheckbox->setChecked(opt.m_fbxAssumeMetallic);
        ConversionTab->DeduplicateMaterialsCheckbox->setChecked(opt.m_deduplicateMaterials);
        ConversionTab->Axis0Combo->setCurrentIndex((int)opt.m_axis1);
        ConversionTab->Axis1Combo->setCurrentIndex((int)opt.m_axis2);
        ConversionTab->Axis2Combo->setCurrentIndex((int)opt.m_axis3);
        ConversionTab->VertexPositionCombo->setCurrentIndex((int)opt.m_vertexPosition);
        ConversionTab->VertexColor0Combo->setCurrentIndex((int)opt.m_vertexColor0);
        ConversionTab->VertexColor1Combo->setCurrentIndex((int)opt.m_vertexColor1);
        ConversionTab->VertexNormalCombo->setCurrentIndex((int)opt.m_vertexNormal);
        ConversionTab->VertexTangentCombo->setCurrentIndex((int)opt.m_vertexTangent);
        ConversionTab->VertexBitangentCombo->setCurrentIndex((int)opt.m_vertexBinormal);
        ConversionTab->TexCoord0Combo->setCurrentIndex((int)opt.m_vertexTexCoord0);
        ConversionTab->TexCoord1Combo->setCurrentIndex((int)opt.m_vertexTexCoord1);

        const auto flags = conv.m_availableOptions;

        ConversionTab->ScalingSpinbox->setVisible(flags & (uint64_t)ConversionOption::UniformScaling);
        ConversionTab->ScalingSpinboxL->setVisible(flags & (uint64_t)ConversionOption::UniformScaling);
        ConversionTab->RecenterToOriginCheckbox->setVisible(flags & (uint64_t)ConversionOption::RecenterToOrigin);
        ConversionTab->RecenterToOriginCheckboxL->setVisible(flags & (uint64_t)ConversionOption::RecenterToOrigin);
        ConversionTab->DefaultSidednessCombo->setVisible(flags & (uint64_t)ConversionOption::MaterialDefaultSidedness);
        ConversionTab->DefaultSidednessComboL->setVisible(flags & (uint64_t)ConversionOption::MaterialDefaultSidedness);
        ConversionTab->GammaToLinearMaterialCombo->setVisible(flags & (uint64_t)ConversionOption::GammaToLinearMaterial);
        ConversionTab->GammaToLinearMaterialCheckboxL->setVisible(flags & (uint64_t)ConversionOption::GammaToLinearMaterial);
        ConversionTab->GammaToLinearVertexCombo->setVisible(flags & (uint64_t)ConversionOption::GammaToLinearVertex);
        ConversionTab->GammaToLinearVertexCheckboxL->setVisible(flags & (uint64_t)ConversionOption::GammaToLinearVertex);
        ConversionTab->ScenegraphModeCombo->setVisible(flags & (uint64_t)ConversionOption::SceneGraphMode);
        ConversionTab->ScenegraphModeComboL->setVisible(flags & (uint64_t)ConversionOption::SceneGraphMode);
        ConversionTab->CollisionMeshCheckbox->setVisible(flags & (uint64_t)ConversionOption::CollisionMesh);
        ConversionTab->CollisionMeshCheckboxL->setVisible(flags & (uint64_t)ConversionOption::CollisionMesh);
        ConversionTab->UnlitMaterialsCheckbox->setVisible(flags & (uint64_t)ConversionOption::UnlitMaterials);
        ConversionTab->UnlitMaterialsCheckboxL->setVisible(flags & (uint64_t)ConversionOption::UnlitMaterials);
        ConversionTab->FbxAssumeMetallicCheckbox->setVisible(flags & (uint64_t)ConversionOption::FbxAssumeMetallic);
        ConversionTab->FbxAssumeMetallicCheckboxL->setVisible(flags & (uint64_t)ConversionOption::FbxAssumeMetallic);
        ConversionTab->DeduplicateMaterialsCheckbox->setVisible(flags & (uint64_t)ConversionOption::DeduplicateMaterials);
        ConversionTab->DeduplicateMaterialsCheckboxL->setVisible(flags & (uint64_t)ConversionOption::DeduplicateMaterials);
        ConversionTab->Axis0Combo->setVisible(flags & (uint64_t)ConversionOption::AxisMapping);
        ConversionTab->Axis0ComboL->setVisible(flags & (uint64_t)ConversionOption::AxisMapping);
        ConversionTab->Axis1Combo->setVisible(flags & (uint64_t)ConversionOption::AxisMapping);
        ConversionTab->Axis1ComboL->setVisible(flags & (uint64_t)ConversionOption::AxisMapping);
        ConversionTab->Axis2Combo->setVisible(flags & (uint64_t)ConversionOption::AxisMapping);
        ConversionTab->Axis2ComboL->setVisible(flags & (uint64_t)ConversionOption::AxisMapping);
        ConversionTab->VertexPositionCombo->setVisible(flags & (uint64_t)ConversionOption::VertexPositionFormat);
        ConversionTab->VertexPositionComboL->setVisible(flags & (uint64_t)ConversionOption::VertexPositionFormat);
        ConversionTab->VertexColor0Combo->setVisible(flags & (uint64_t)ConversionOption::VertexColor0Format);
        ConversionTab->VertexColor0ComboL->setVisible(flags & (uint64_t)ConversionOption::VertexColor0Format);
        ConversionTab->VertexColor1Combo->setVisible(flags & (uint64_t)ConversionOption::VertexColor1Format);
        ConversionTab->VertexColor1ComboL->setVisible(flags & (uint64_t)ConversionOption::VertexColor1Format);
        ConversionTab->VertexNormalCombo->setVisible(flags & (uint64_t)ConversionOption::VertexNormalFormat);
        ConversionTab->VertexNormalComboL->setVisible(flags & (uint64_t)ConversionOption::VertexNormalFormat);
        ConversionTab->VertexTangentCombo->setVisible(flags & (uint64_t)ConversionOption::VertexTangentFormat);
        ConversionTab->VertexTangentComboL->setVisible(flags & (uint64_t)ConversionOption::VertexTangentFormat);
        ConversionTab->VertexBitangentCombo->setVisible(flags & (uint64_t)ConversionOption::VertexBinormalFormat);
        ConversionTab->VertexBitangentComboL->setVisible(flags & (uint64_t)ConversionOption::VertexBinormalFormat);
        ConversionTab->TexCoord0Combo->setVisible(flags & (uint64_t)ConversionOption::VertexTexCoord0Format);
        ConversionTab->TexCoord0ComboL->setVisible(flags & (uint64_t)ConversionOption::VertexTexCoord0Format);
        ConversionTab->TexCoord1Combo->setVisible(flags & (uint64_t)ConversionOption::VertexTexCoord1Format);
        ConversionTab->TexCoord1ComboL->setVisible(flags & (uint64_t)ConversionOption::VertexTexCoord1Format);
    }

    ConversionTab->ConversionList->setCurrentRow(m_conversionManager->GetSelectedConversionIndex());
}

void ArrtAppWindow::RetrieveConversionOptions()
{
    if (!m_conversionManager->IsEditableSelected())
        return;

    m_conversionManager->blockSignals(true);

    m_conversionManager->SetConversionName(ConversionTab->ConversionNameInput->text());

    ConversionOptions opt;
    opt.m_scaling = (float)ConversionTab->ScalingSpinbox->value();
    opt.m_recenterToOrigin = ConversionTab->RecenterToOriginCheckbox->isChecked();
    opt.m_opaqueMaterialDefaultSidedness = (Sideness)ConversionTab->DefaultSidednessCombo->currentIndex();
    opt.m_materialColorSpace = (ColorSpaceMode)ConversionTab->GammaToLinearMaterialCombo->currentIndex();
    opt.m_vertexColorSpace = (ColorSpaceMode)ConversionTab->GammaToLinearVertexCombo->currentIndex();
    opt.m_sceneGraphMode = (SceneGraphMode)ConversionTab->ScenegraphModeCombo->currentIndex();
    opt.m_generateCollisionMesh = ConversionTab->CollisionMeshCheckbox->isChecked();
    opt.m_unlitMaterials = ConversionTab->UnlitMaterialsCheckbox->isChecked();
    opt.m_fbxAssumeMetallic = ConversionTab->FbxAssumeMetallicCheckbox->isChecked();
    opt.m_deduplicateMaterials = ConversionTab->DeduplicateMaterialsCheckbox->isChecked();
    opt.m_axis1 = (Axis)ConversionTab->Axis0Combo->currentIndex();
    opt.m_axis2 = (Axis)ConversionTab->Axis1Combo->currentIndex();
    opt.m_axis3 = (Axis)ConversionTab->Axis2Combo->currentIndex();
    opt.m_vertexPosition = (VertexPosition)ConversionTab->VertexPositionCombo->currentIndex();
    opt.m_vertexColor0 = (VertexColor)ConversionTab->VertexColor0Combo->currentIndex();
    opt.m_vertexColor1 = (VertexColor)ConversionTab->VertexColor1Combo->currentIndex();
    opt.m_vertexNormal = (VertexVector)ConversionTab->VertexNormalCombo->currentIndex();
    opt.m_vertexTangent = (VertexVector)ConversionTab->VertexTangentCombo->currentIndex();
    opt.m_vertexBinormal = (VertexVector)ConversionTab->VertexBitangentCombo->currentIndex();
    opt.m_vertexTexCoord0 = (VertexTextureCoord)ConversionTab->TexCoord0Combo->currentIndex();
    opt.m_vertexTexCoord1 = (VertexTextureCoord)ConversionTab->TexCoord1Combo->currentIndex();

    m_conversionManager->SetConversionAdvancedOptions(opt);

    m_conversionManager->blockSignals(false);
}

void ArrtAppWindow::on_ConversionOptionsCheckbox_stateChanged(int state)
{
    RetrieveConversionOptions();

    m_conversionManager->SetConversionAdvanced(state == Qt::Checked);
}

void ArrtAppWindow::UpdateConversionStartButton()
{
    const Conversion& conv = m_conversionManager->GetSelectedConversion();
    if (conv.m_status != ConversionStatus::New || conv.m_sourceAsset.isEmpty() || conv.m_outputFolder.isEmpty())
    {
        ConversionTab->StartConversionButton->setEnabled(false);
        return;
    }

    ConversionTab->StartConversionButton->setEnabled(true);
}

void ArrtAppWindow::on_ResetAdvancedButton_clicked()
{
    if (!m_conversionManager->IsEditableSelected())
        return;

    m_conversionManager->SetConversionAdvancedOptions(ConversionOptions());
}