#include <Conversion/ConversionManager.h>
#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QPointer>
#include <Rendering/ArrAccount.h>
#include <Rendering/IncludeAzureRemoteRendering.h>
#include <Storage/StorageAccount.h>

ConversionManager::ConversionManager(StorageAccount* storageAccount, ArrAccount* arrClient)
{
    m_storageAccount = storageAccount;
    m_arrClient = arrClient;
    m_conversions.resize(1);

    m_checkConversionStateTimer.setInterval(10000);
    connect(&m_checkConversionStateTimer, &QTimer::timeout, this, &ConversionManager::onCheckConversions);

    m_updateConversionListTimer.setInterval(1000);
    connect(&m_updateConversionListTimer, &QTimer::timeout, this, [this]()
            { Q_EMIT ListChanged(); });
}

ConversionManager::~ConversionManager() = default;

uint32_t ConversionManager::GetNumActiveConversions() const
{
    uint32_t active = 0;

    for (const auto& conv : m_conversions)
    {
        if (conv.m_status == ConversionStatus::Running)
        {
            ++active;
        }
    }

    return active;
}

void ConversionManager::SetSelectedConversion(int selected)
{
    if (selected < 0)
        return;

    m_selectedConversion = selected;

    Q_EMIT SelectedChanged();
}

bool ConversionManager::IsEditableSelected() const
{
    return m_conversions[m_selectedConversion].m_status == ConversionStatus::New;
}

bool ConversionManager::StartConversion()
{
    if (!IsEditableSelected())
        return false;

    auto& conv = m_conversions[m_selectedConversion];

    conv.m_status = ConversionStatus::Running;

    if (conv.m_name.isEmpty())
    {
        conv.m_name = conv.GetPlaceholderName();
    }

    if (!StartConversionInternal())
    {
        conv.m_status = ConversionStatus::New;
        return false;
    }

    Q_EMIT SelectedChanged();

    m_conversions.push_back({});
    Q_EMIT ListChanged();

    return true;
}

void ConversionManager::SetConversionName(const QString& name)
{
    auto& conv = m_conversions[m_selectedConversion];

    if (conv.m_name != name)
    {
        conv.m_name = name;

        Q_EMIT SelectedChanged();
    }
}

void ConversionManager::SetConversionSourceAsset(const QString& container, const QString& path)
{
    auto& conv = m_conversions[m_selectedConversion];

    if (conv.m_sourceAssetContainer != container || conv.m_sourceAsset != path)
    {
        conv.m_sourceAssetContainer = container;
        conv.m_sourceAsset = path;

        Q_EMIT SelectedChanged();
    }
}

void ConversionManager::SetConversionOutputFolder(const QString& container, const QString& path)
{
    auto& conv = m_conversions[m_selectedConversion];

    if (conv.m_outputFolderContainer != container || conv.m_outputFolder != path)
    {
        conv.m_outputFolderContainer = container;
        conv.m_outputFolder = path;

        Q_EMIT SelectedChanged();
    }
}

void ConversionManager::SetConversionAdvanced(bool advanced)
{
    auto& conv = m_conversions[m_selectedConversion];

    if (conv.m_showAdvancedOptions != advanced)
    {
        conv.m_showAdvancedOptions = advanced;
        Q_EMIT SelectedChanged();
    }
}

void ConversionManager::SetConversionAdvancedOptions(const ConversionOptions& options)
{
    auto& conv = m_conversions[m_selectedConversion];

    conv.m_options = options;
    Q_EMIT SelectedChanged();
}

void ConversionManager::onCheckConversions()
{
    bool anyRunning = false;

    for (size_t conversionIdx = 0; conversionIdx < m_conversions.size(); ++conversionIdx)
    {
        auto& conv = m_conversions[conversionIdx];

        if (conv.m_status != ConversionStatus::Running)
            continue;

        anyRunning = true;

        m_arrClient->GetClient()->GetAssetConversionStatusAsync(conv.m_conversionGuid.toStdString(), [this, conversionIdx](RR::Status status, RR::ApiHandle<RR::AssetConversionStatusResult> result)
                                                                { SetConversionStatus((int)conversionIdx, status, result); });
    }

    Q_EMIT ListChanged();

    if (GetSelectedConversion().m_status != ConversionStatus::New)
    {
        Q_EMIT SelectedChanged();
    }

    if (!anyRunning)
    {
        // no need to keep the timer running
        m_checkConversionStateTimer.stop();
        m_updateConversionListTimer.stop();

        QApplication::alert(QApplication::topLevelWidgets()[0], 2000);
    }
}

bool ConversionManager::StartConversionInternal()
{
    auto& conv = m_conversions[m_selectedConversion];

    {
        QString srcFolder = conv.m_sourceAsset;
        int lastSlash = srcFolder.lastIndexOf("/");
        if (lastSlash >= 0)
        {
            srcFolder = srcFolder.left(lastSlash + 1);
        }

        std::vector<StorageAccount::BlobInfo> dirs, files;
        m_storageAccount->ListBlobDirectory(conv.m_sourceAssetContainer, srcFolder, dirs, files);

        int srcAssets = 0;
        for (const auto& file : files)
        {
            if (file.m_path.endsWith(".fbx", Qt::CaseInsensitive) ||
                file.m_path.endsWith(".glb", Qt::CaseInsensitive) ||
                file.m_path.endsWith(".gltf", Qt::CaseInsensitive))
            {
                srcAssets++;
            }
        }

        if (srcAssets > 1)
        {
            if (QMessageBox::warning(nullptr, "Multiple Source Assets Found", QString("The folder of the input asset contains %1 asset files (GLB, GLTF or FBX). The conversion service needs to download the entire folder. The more unrelated data is in that folder, the longer the conversion will take because of this download.\n\nFor best conversion speed, every asset (and its accompanying files, such as textures) should reside in its own folder.\n\nContinue anyway?").arg(srcAssets), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No)
            {
                return false;
            }
        }
    }

    {
        QString advancedOptionsJSON = conv.m_options.ToJSON();

        QFileInfo assetFile = conv.m_sourceAsset;
        QString settingsFileName = assetFile.path() + "/" + assetFile.completeBaseName() + ".ConversionSettings.json";

        QString errorMsg;

        if (!m_storageAccount->CreateTextItem(conv.m_sourceAssetContainer, settingsFileName, advancedOptionsJSON, errorMsg))
        {
            QMessageBox::critical(nullptr, "Starting Conversion Failed", QString("Could not upload the ConversionSettings.json file to the blob storage.\n\nReason: %1").arg(errorMsg), QMessageBox::StandardButton::Ok);
            return false;
        }
    }

    const auto inputContainerUri = m_storageAccount->GetContainerUriFromName(conv.m_sourceAssetContainer);
    const auto outputContainerUri = m_storageAccount->GetContainerUriFromName(conv.m_outputFolderContainer);

    const QString inputSasToken = m_storageAccount->GetSasToken(inputContainerUri, azure::storage::blob_shared_access_policy::read | azure::storage::blob_shared_access_policy::list, 60 * 24);

    const QString outputSasToken = m_storageAccount->GetSasToken(outputContainerUri, azure::storage::blob_shared_access_policy::write | azure::storage::blob_shared_access_policy::list | azure::storage::blob_shared_access_policy::create, 60 * 24);

    const QString inputUri = QString("https://%1.blob.core.windows.net/%2").arg(m_storageAccount->GetAccountName()).arg(conv.m_sourceAssetContainer);
    const QString outputUri = QString("https://%1.blob.core.windows.net/%2").arg(m_storageAccount->GetAccountName()).arg(conv.m_outputFolderContainer);

    const QString relInputPath = QFileInfo(conv.m_sourceAsset).dir().path();
    const QString relInputFile = QFileInfo(conv.m_sourceAsset).fileName();

    const QString relOutputPath = conv.m_outputFolder;
    const QString relOutputFile = conv.m_name + ".arrAsset";

    RR::AssetConversionOptions options;

    RR::AssetConversionInputOptions& input(options.InputOptions);
    input.BlobPrefix = relInputPath.toStdString();
    input.RelativeInputAssetPath = relInputFile.toStdString();
    input.StorageContainerReadListSas = inputSasToken.toStdString();
    input.StorageContainerUri = inputUri.toStdString();

    RR::AssetConversionOutputOptions& output(options.OutputOptions);
    output.BlobPrefix = relOutputPath.toStdString();
    output.OutputAssetFilename = relOutputFile.toStdString();
    output.StorageContainerWriteSas = outputSasToken.toStdString();
    output.StorageContainerUri = outputUri.toStdString();

    int conversionIdx = m_selectedConversion;

    auto onConversionStartRequestFinished = [this, conversionIdx](RR::Status status, RR::ApiHandle<RR::AssetConversionResult> result)
    {
        QMetaObject::invokeMethod(QApplication::instance(), [this, conversionIdx, status, result]()
                                  {
                                      std::string conversionUUID;
                                      RR::Result errorCode = RR::StatusToResult(status);

                                      if (status == RR::Status::OK)
                                      {
                                          errorCode = result->GetErrorCode();
                                          result->GetConversionUuid(conversionUUID);
                                      }

                                      auto& conv = m_conversions[conversionIdx];
                                      conv.m_conversionGuid = conversionUUID.c_str();

                                      if (errorCode != RR::Result::Success)
                                      {
                                          conv.m_message = "Starting conversion failed";
                                          conv.m_status = ConversionStatus::Failed;
                                          conv.m_endConversionTime = QDateTime::currentSecsSinceEpoch();
                                      }
                                  });
    };

    m_arrClient->GetClient()->StartAssetConversionAsync(options, onConversionStartRequestFinished);

    m_checkConversionStateTimer.start();
    m_updateConversionListTimer.start();

    conv.m_startConversionTime = QDateTime::currentSecsSinceEpoch();
    conv.m_endConversionTime = conv.m_startConversionTime;

    return true;
}

void ConversionManager::SetConversionStatus(int conversionIdx, RR::Status status, RR::ApiHandle<RR::AssetConversionStatusResult> result)
{
    auto& conv = m_conversions[conversionIdx];

    std::string message;
    RR::ConversionSessionStatus conversionResult;

    if (status == RR::Status::OK)
    {
        result->GetErrorMessage(message);
        conversionResult = result->GetResult();
    }
    else
    {
        conversionResult = RR::ConversionSessionStatus::Failure;
        message = "Unknown failure";
    }

    switch (conversionResult)
    {
        case RR::ConversionSessionStatus::Unknown:
        case RR::ConversionSessionStatus::Created:
        case RR::ConversionSessionStatus::Running:
            conv.m_status = ConversionStatus::Running;
            break;

        case RR::ConversionSessionStatus::Aborted:
        case RR::ConversionSessionStatus::Failure:
            conv.m_status = ConversionStatus::Failed;
            conv.m_message = message.c_str();
            conv.m_endConversionTime = QDateTime::currentSecsSinceEpoch();
            break;

        case RR::ConversionSessionStatus::Success:
            conv.m_status = ConversionStatus::Finished;
            conv.m_endConversionTime = QDateTime::currentSecsSinceEpoch();
            break;
    }
}

QString Conversion::GetPlaceholderName() const
{
    return QFileInfo(m_sourceAsset).baseName();
}
