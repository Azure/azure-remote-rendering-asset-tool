// for weird reasons, if this isn't included before App/AppWindow.h in this file, there is a compiler error (probably preprocessor polution)
#include <Rendering/IncludeAzureRemoteRendering.h>

#include <App/AppWindow.h>
#include <QInputDialog>
#include <QMessageBox>
#include <Rendering/ArrSession.h>
#include <Rendering/UI/SceneState.h>
#include <Rendering/UI/ScenegraphModel.h>
#include <Rendering/UI/StartSessionDlg.h>
#include <Rendering/Ui/CameraDlg.h>
#include <Storage/StorageAccount.h>
#include <Storage/UI/BrowseStorageDlg.h>

bool IsVideoDecoderAvailable(std::string& details);

void ArrtAppWindow::on_ChangeModelButton_clicked()
{
    BrowseStorageDlg dlg(m_storageAccount.get(), StorageEntry::Type::ArrAsset, m_lastStorageLoadModelContainer, QString(), this, "Load from storage...");
    if (dlg.exec() == QDialog::Rejected)
        return;

    m_lastStorageLoadModelContainer = dlg.GetSelectedContainer();

    QString blobUri = m_storageAccount->CreateSasURL(m_lastStorageLoadModelContainer, dlg.GetSelectedItem());
    m_arrSession->LoadModel(dlg.GetSelectedItem(), blobUri.toStdString());
}

void ArrtAppWindow::on_LoadModelLinkedAccountButton_clicked()
{
    BrowseStorageDlg dlg(m_storageAccount.get(), StorageEntry::Type::ArrAsset, m_lastStorageLoadModelContainer, QString(), this, "Load from storage linked to ARR account...");
    if (dlg.exec() == QDialog::Rejected)
        return;

    m_lastStorageLoadModelContainer = dlg.GetSelectedContainer();

    std::string blobUri;
    m_arrSession->LoadModel(dlg.GetSelectedItem(), blobUri, m_storageAccount->GetEndpointUrl(), m_lastStorageLoadModelContainer);
}

void ArrtAppWindow::on_ClearModelsButton_clicked()
{
    while (m_arrSession->GetLoadedModels().size() > 0)
    {
        m_arrSession->RemoveModel(0);
    }
}

void ArrtAppWindow::on_LoadModelSasButton_clicked()
{
    QString url = QInputDialog::getText(this, "Input SAS URL", "Please provide the SAS URL for the model that you want to load.", QLineEdit::Normal, "builtin://Engine");

    if (url.isEmpty())
        return;

    m_arrSession->LoadModel(url, url.toUtf8().data());
}

void ArrtAppWindow::on_EditSessionButton_clicked()
{
    std::string details;
    if (!IsVideoDecoderAvailable(details))
    {
        QMessageBox::critical(this, "Missing Video Decoder", QString("<html>This PC does not meet the minimum requirements to display a remotely rendered video stream.<br><br>Error: '") + QString(details.c_str()) + QString("'<br><br>Please <a href=\"https://learn.microsoft.com/azure/remote-rendering/overview/system-requirements#devices\">check the documentation</a> for details.</html>"));
        return;
    }

    StartSessionDlg dlg(m_arrSession.get(), m_arrSettings.get(), this);
    dlg.exec();
}

void ArrtAppWindow::on_InspectorButton_clicked()
{
    m_arrSession->StartArrInspector();
}

void ArrtAppWindow::OnEntitySelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    for (const QModelIndex& index : selected.indexes())
    {
        m_arrSession->SetEntitySelected(m_scenegraphModel->GetEntityHandle(index), true);
    }

    for (const QModelIndex& index : deselected.indexes())
    {
        m_arrSession->SetEntitySelected(m_scenegraphModel->GetEntityHandle(index), false);
    }

    UpdateMaterialsList();
}

void ArrtAppWindow::OnEntityPicked()
{
    auto entity = m_sceneState->GetLastPickedEntity();

    if (entity && entity->GetValid())
    {
        auto index = m_scenegraphModel->MapArrEntityHandleToModelIndex(entity);

        RenderingTab->ScenegraphView->selectionModel()->select(index, QItemSelectionModel::SelectionFlag::ClearAndSelect);

        auto parent = m_scenegraphModel->parent(index);
        while (parent.isValid())
        {
            RenderingTab->ScenegraphView->setExpanded(parent, true);
            parent = m_scenegraphModel->parent(parent);
        }

        RenderingTab->ScenegraphView->scrollTo(index);
    }
    else
    {
        // deselect everything that was selected so far
        RenderingTab->ScenegraphView->selectionModel()->clearSelection();
    }
}

void ArrtAppWindow::on_CameraOptionsButton_clicked()
{
    CameraDlg dlg(m_arrSettings.get(), this);
    dlg.exec();
}

void ArrtAppWindow::UpdateFrameStatisticsUI()
{
    const auto [frames, stats] = m_arrSession->GetFrameStatistics();

    RenderingTab->FramesPerSecondLabel->setText(QString("Render: %1fps").arg((size_t)(frames)));
    RenderingTab->LatencyPoseToReceiveLabel->setText(QString("Latency Pose To Receive: %1ms").arg((int)(stats.LatencyPoseToReceive * 1000)));
    RenderingTab->LatencyReceiveToPresentLabel->setText(QString("Latency Receive To Present: %1ms").arg((int)(stats.LatencyReceiveToPresent * 1000)));
    RenderingTab->LatencyPresentToDisplayLabel->setText(QString("Latency Present To Display: %1ms").arg((int)(stats.LatencyPresentToDisplay * 1000)));
    RenderingTab->TimeSinceLastPresentLabel->setText(QString("Time Since Last Present: %1ms").arg((int)(stats.TimeSinceLastPresent * 1000)));
    RenderingTab->VideoFrameReusedCountLabel->setText(QString("Video Frame Reused Count: %1").arg(stats.VideoFrameReusedCount));
    RenderingTab->VideoFramesSkippedLabel->setText(QString("Video Frames Skipped: %1").arg(stats.VideoFramesSkipped));
    RenderingTab->VideoFramesReceivedLabel->setText(QString("Video Frames Received: %1").arg(stats.VideoFramesReceived));
    RenderingTab->VideoFramesDiscardedLabel->setText(QString("Video Frames Discarded: %1").arg(stats.VideoFramesDiscarded));
    RenderingTab->VideoFrameMinDeltaLabel->setText(QString("Video Frame Min Delta: %1ms").arg((int)(stats.VideoFrameMinDelta * 1000)));
    RenderingTab->VideoFrameMaxDeltaLabel->setText(QString("Video Frame Max Delta: %1ms").arg((int)(stats.VideoFrameMaxDelta * 1000)));
}

void ArrtAppWindow::OnEntityDoubleClicked(const QModelIndex& index)
{
    if (index.isValid())
    {
        m_sceneState->FocusOnEntity(m_scenegraphModel->GetEntityHandle(index));
    }
}

void ArrtAppWindow::on_ModelScaleSpinner_valueChanged(double d)
{
    if (m_arrSession)
    {
        m_arrSession->SetModelScale((float)d);
    }
}
