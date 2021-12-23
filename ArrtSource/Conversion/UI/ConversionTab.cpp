#include <App/AppWindow.h>
#include <QDir>
#include <Storage/UI/BrowseStorageDlg.h>

void ArrtAppWindow::on_ConversionList_currentRowChanged(int row)
{
    RetrieveConversionOptions();
    m_conversionManager->SetSelectedConversion(row);
    UpdateConversionPane();
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

    if (ConversionList->count() > conversions.size())
    {
        // an element got removed -> clear the entire list and rebuild it
        ConversionList->clear();
    }

    if (ConversionList->count() < conversions.size())
    {
        // an element got added (we always append) -> add the new ones

        for (int i = ConversionList->count(); i < (int)conversions.size(); ++i)
        {
            const Conversion& conv = conversions[i];

            new QListWidgetItem(conv.m_name, ConversionList);
        }
    }

    // update the display of all items
    for (int i = 0; i < ConversionList->count(); ++i)
    {
        const Conversion& conv = conversions[i];
        QListWidgetItem* item = ConversionList->item(i);


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
                text += QString(" (succeeded) [%1]").arg(SecToString(duration));
                break;
            }
            case ConversionStatus::Failed:
            {
                const uint64_t duration = conv.m_endConversionTime - conv.m_startConversionTime;
                item->setIcon(QIcon(":/ArrtApplication/Icons/conversion_failed.svg"));
                text += QString(" (failed) [%1]").arg(SecToString(duration));
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
        ConversionNameInput->setReadOnly(!allowEditing);
        SelectSourceButton->setEnabled(allowEditing);
        SelectOutputFolderButton->setEnabled(allowEditing);
        SelectInputFolderButton->setEnabled(allowEditing && !conv.m_sourceAsset.isEmpty());
        OptionsArea->setEnabled(allowEditing);

        // enable or disable the start conversion button depending on whether enough data is set
        StartConversionButton->setEnabled(allowEditing && !conv.m_sourceAsset.isEmpty() && !conv.m_outputFolder.isEmpty());
        ResetAdvancedButton->setEnabled(allowEditing);
    }

    // general state
    {
        ConversionNameInput->setText(conv.m_name);
        SourceAssetLine->setText(conv.m_sourceAssetContainer + ":" + conv.m_sourceAsset);
        OutputFolderLine->setText(conv.m_outputFolderContainer + ":" + conv.m_outputFolder);

        if (conv.m_inputFolder.isEmpty())
            InputFolderLine->setText({});
        else
            InputFolderLine->setText(conv.m_sourceAssetContainer + ":" + conv.m_inputFolder);

        ConversionOptionsCheckbox->blockSignals(true);
        ConversionOptionsCheckbox->setChecked(conv.m_showAdvancedOptions);
        ConversionOptionsCheckbox->blockSignals(false);
        ConversionNameInput->setPlaceholderText(conv.GetPlaceholderName());
        InputFolderLine->setPlaceholderText(conv.m_sourceAssetContainer + ":" + conv.GetPlaceholderInputFolder());
    }

    if (conv.m_message.isEmpty())
    {
        switch (conv.m_status)
        {
            case ConversionStatus::New:
                ConversionMessage->setText("Conversion Status: not started");
                break;
            case ConversionStatus::Finished:
                ConversionMessage->setText("Conversion Status: finished successfully");
                break;
            case ConversionStatus::Running:
                ConversionMessage->setText("Conversion Status: currently running");
                break;
            case ConversionStatus::Failed:
                ConversionMessage->setText("Conversion Status: failed (no details)");
                break;
        }
    }
    else
    {
        ConversionMessage->setText(QString("Conversion Status: %1").arg(conv.m_message));
    }

    // show advanced options
    {
        if (conv.m_showAdvancedOptions)
        {
            OptionsSpacer->changeSize(0, 0, QSizePolicy::Ignored, QSizePolicy::Ignored);
        }
        else
        {
            OptionsSpacer->changeSize(0, 0, QSizePolicy::Ignored, QSizePolicy::Expanding);
        }

        OptionsArea->setVisible(conv.m_showAdvancedOptions);
    }

    // set advanced options
    {
        const auto& opt = conv.m_options;

        ScalingSpinbox->setValue(opt.m_scaling);
        RecenterToOriginCheckbox->setChecked(opt.m_recenterToOrigin);
        DefaultSidednessCombo->setCurrentIndex((int)opt.m_opaqueMaterialDefaultSidedness);
        MaterialOverrideEdit->setText(opt.m_materialOverride);
        GammaToLinearMaterialCheckbox->setChecked(opt.m_gammaToLinearMaterial);
        GammaToLinearVertexCheckbox->setChecked(opt.m_gammaToLinearVertex);
        ScenegraphModeCombo->setCurrentIndex((int)opt.m_sceneGraphMode);
        CollisionMeshCheckbox->setChecked(opt.m_generateCollisionMesh);
        UnlitMaterialsCheckbox->setChecked(opt.m_unlitMaterials);
        FbxAssumeMetallicCheckbox->setChecked(opt.m_fbxAssumeMetallic);
        DeduplicateMaterialsCheckbox->setChecked(opt.m_deduplicateMaterials);
        Axis0Combo->setCurrentIndex((int)opt.m_axis1);
        Axis1Combo->setCurrentIndex((int)opt.m_axis2);
        Axis2Combo->setCurrentIndex((int)opt.m_axis3);
        VertexPositionCombo->setCurrentIndex((int)opt.m_vertexPosition);
        VertexColor0Combo->setCurrentIndex((int)opt.m_vertexColor0);
        VertexColor1Combo->setCurrentIndex((int)opt.m_vertexColor1);
        VertexNormalCombo->setCurrentIndex((int)opt.m_vertexNormal);
        VertexTangentCombo->setCurrentIndex((int)opt.m_vertexTangent);
        VertexBitangentCombo->setCurrentIndex((int)opt.m_vertexBinormal);
        TexCoord0Combo->setCurrentIndex((int)opt.m_vertexTexCoord0);
        TexCoord1Combo->setCurrentIndex((int)opt.m_vertexTexCoord1);
    }
}

void ArrtAppWindow::RetrieveConversionOptions()
{
    if (!m_conversionManager->IsEditableSelected())
        return;

    m_conversionManager->SetConversionName(ConversionNameInput->text());

    ConversionOptions opt;
    opt.m_scaling = (float)ScalingSpinbox->value();
    opt.m_recenterToOrigin = RecenterToOriginCheckbox->isChecked();
    opt.m_opaqueMaterialDefaultSidedness = (Sideness)DefaultSidednessCombo->currentIndex();
    opt.m_materialOverride = MaterialOverrideEdit->text();
    opt.m_gammaToLinearMaterial = GammaToLinearMaterialCheckbox->isChecked();
    opt.m_gammaToLinearVertex = GammaToLinearVertexCheckbox->isChecked();
    opt.m_sceneGraphMode = (SceneGraphMode)ScenegraphModeCombo->currentIndex();
    opt.m_generateCollisionMesh = CollisionMeshCheckbox->isChecked();
    opt.m_unlitMaterials = UnlitMaterialsCheckbox->isChecked();
    opt.m_fbxAssumeMetallic = FbxAssumeMetallicCheckbox->isChecked();
    opt.m_deduplicateMaterials = DeduplicateMaterialsCheckbox->isChecked();
    opt.m_axis1 = (Axis)Axis0Combo->currentIndex();
    opt.m_axis2 = (Axis)Axis1Combo->currentIndex();
    opt.m_axis3 = (Axis)Axis2Combo->currentIndex();
    opt.m_vertexPosition = (VertexPosition)VertexPositionCombo->currentIndex();
    opt.m_vertexColor0 = (VertexColor)VertexColor0Combo->currentIndex();
    opt.m_vertexColor1 = (VertexColor)VertexColor1Combo->currentIndex();
    opt.m_vertexNormal = (VertexVector)VertexNormalCombo->currentIndex();
    opt.m_vertexTangent = (VertexVector)VertexTangentCombo->currentIndex();
    opt.m_vertexBinormal = (VertexVector)VertexBitangentCombo->currentIndex();
    opt.m_vertexTexCoord0 = (VertexTextureCoord)TexCoord0Combo->currentIndex();
    opt.m_vertexTexCoord1 = (VertexTextureCoord)TexCoord1Combo->currentIndex();

    m_conversionManager->SetConversionAdvancedOptions(opt);
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
        StartConversionButton->setEnabled(false);
        return;
    }

    StartConversionButton->setEnabled(true);
}

void ArrtAppWindow::on_ResetAdvancedButton_clicked()
{
    if (!m_conversionManager->IsEditableSelected())
        return;

    m_conversionManager->SetConversionAdvancedOptions(ConversionOptions());
}