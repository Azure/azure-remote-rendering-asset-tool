// for weird reasons, if this isn't included before App/AppWindow.h in this file, there is a compiler error (probably preprocessor polution)
#include <Rendering/IncludeAzureRemoteRendering.h>

#include <App/AppWindow.h>
#include <QInputDialog>
#include <Rendering/ArrSession.h>
#include <Rendering/UI/SceneState.h>
#include <Rendering/UI/ScenegraphModel.h>
#include <Rendering/UI/StartSessionDlg.h>
#include <Rendering/Ui/CameraDlg.h>
#include <Storage/StorageAccount.h>
#include <Storage/UI/BrowseStorageDlg.h>

void ArrtAppWindow::on_ChangeModelButton_clicked()
{
    BrowseStorageDlg dlg(m_storageAccount.get(), StorageEntry::Type::ArrAsset, m_lastStorageLoadModelContainer, this);
    if (dlg.exec() == QDialog::Rejected)
        return;

    m_lastStorageLoadModelContainer = dlg.GetSelectedContainer();

    auto uri = dlg.GetSelectedUri();

    if (!uri.primary_uri().is_empty())
    {
        QString blobUri = m_storageAccount->GetSasUrl(uri);

        m_arrSession->LoadModel(dlg.GetSelectedItem(), blobUri.toStdString().c_str());
    }
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
    StartSessionDlg dlg(m_arrSession.get(), m_arrSettings.get(), this);
    dlg.exec();
}

void ArrtAppWindow::on_InspectorButton_clicked()
{
    m_arrSession->StartArrInspector();
}

void ArrtAppWindow::onEntitySelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    for (const QModelIndex& index : selected.indexes())
    {
        m_arrSession->EnableSelectionOutline(m_scenegraphModel->GetEntityHandle(index), true);
    }

    for (const QModelIndex& index : deselected.indexes())
    {
        m_arrSession->EnableSelectionOutline(m_scenegraphModel->GetEntityHandle(index), false);
    }

    UpdateMaterialsList();
}

void ArrtAppWindow::onEntityPicked()
{
    auto entity = m_sceneState->GetLastPickedEntity();

    if (entity && entity->GetValid())
    {
        auto index = m_scenegraphModel->MapArrEntityHandleToModelIndex(entity);

        ScenegraphView->selectionModel()->select(index, QItemSelectionModel::SelectionFlag::ClearAndSelect);
    }
    else
    {
        // deselect everything that was selected so far
        ScenegraphView->selectionModel()->clearSelection();
    }
}

void ArrtAppWindow::on_CameraOptionsButton_clicked()
{
    CameraDlg dlg(m_arrSettings.get(), this);
    dlg.exec();
}

void ArrtAppWindow::UpdateFrameStatisticsUI()
{
    const auto& stats = m_arrSession->GetFrameStatistics();

    LatencyPoseToReceiveLabel->setText(QString("Latency Pose To Receive: %1ms").arg((int)(stats.LatencyPoseToReceive * 1000)));
    LatencyReceiveToPresentLabel->setText(QString("Latency Receive To Present: %1ms").arg((int)(stats.LatencyReceiveToPresent * 1000)));
    TimeSinceLastPresentLabel->setText(QString("Time Since Last Present: %1ms").arg((int)(stats.TimeSinceLastPresent * 1000)));
    VideoFrameReusedCountLabel->setText(QString("Video Frame Reused Count: %1").arg(stats.VideoFrameReusedCount));
    VideoFramesSkippedLabel->setText(QString("Video Frames Skipped: %1").arg(stats.VideoFramesSkipped));
    VideoFramesReceivedLabel->setText(QString("Video Frames Received: %1").arg(stats.VideoFramesReceived));
    VideoFramesDiscardedLabel->setText(QString("Video Frames Discarded: %1").arg(stats.VideoFramesDiscarded));
    VideoFrameMinDeltaLabel->setText(QString("Video Frame Min Delta: %1ms").arg((int)(stats.VideoFrameMinDelta * 1000)));
    VideoFrameMaxDeltaLabel->setText(QString("Video Frame Max Delta: %1ms").arg((int)(stats.VideoFrameMaxDelta * 1000)));
}

void ArrtAppWindow::onEntityDoubleClicked(const QModelIndex& index)
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
