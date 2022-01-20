#include <Conversion/ConversionManager.h>
#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QPointer>
#include <QUuid>
#include <Rendering/ArrAccount.h>
#include <Rendering/IncludeAzureRemoteRendering.h>
#include <Storage/StorageAccount.h>
#include <Storage/UI/StorageBrowserModel.h>
#include <Utils/Logging.h>

ConversionManager::ConversionManager(StorageAccount* storageAccount, ArrAccount* arrClient)
{
    m_storageAccount = storageAccount;
    m_arrClient = arrClient;
    m_conversions.resize(1);

    m_checkConversionStateTimer.setInterval(10000);
    connect(&m_checkConversionStateTimer, &QTimer::timeout, this, &ConversionManager::OnCheckConversions);

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

    if (conv.m_name.isEmpty())
    {
        conv.m_name = conv.GetPlaceholderName();
    }

    if (!StartConversionInternal())
    {
        conv.m_status = ConversionStatus::New;
        Q_EMIT SelectedChanged();
        return false;
    }

    conv.m_status = ConversionStatus::Running;
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
        conv.m_inputFolder = conv.GetPlaceholderInputFolder(); // reset the input folder

        Q_EMIT SelectedChanged();
    }
}

void ConversionManager::SetConversionInputFolder(const QString& path)
{
    auto& conv = m_conversions[m_selectedConversion];

    if (conv.m_inputFolder != path)
    {
        conv.m_inputFolder = path;

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

void ConversionManager::OnCheckConversions()
{
    bool anyRunning = false;

    for (size_t conversionIdx = 0; conversionIdx < m_conversions.size(); ++conversionIdx)
    {
        auto& conv = m_conversions[conversionIdx];

        if (conv.m_status != ConversionStatus::Running || conv.m_conversionGuid.isEmpty())
            continue;

        anyRunning = true;

        m_arrClient->GetClient()->GetAssetConversionStatusAsync(conv.m_conversionGuid.toStdString(), [this, conversionIdx](RR::Status status, RR::ApiHandle<RR::AssetConversionStatusResult> result)
                                                                { QMetaObject::invokeMethod(QApplication::instance(), [this, conversionIdx, status, result]()
                                                                                            { SetConversionStatus((int)conversionIdx, status, result); }); });
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
        std::deque<QString> folders;
        folders.push_back(conv.m_inputFolder);
        int srcAssets = 0;

        while (!folders.empty())
        {
            QString srcFolder = folders.front();
            folders.pop_front();

            std::vector<StorageBlobInfo> dirs, files;
            m_storageAccount->ListBlobDirectory(conv.m_sourceAssetContainer, srcFolder, dirs, files);

            for (const auto& file : files)
            {
                if (StorageBrowserModel::IsSrcAsset(file.m_path))
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
                else
                {
                    // asked once, don't ask again
                    break;
                }
            }

            for (const auto& dir : dirs)
            {
                folders.push_back(dir.m_path);
            }
        }
    }

    {
        QString advancedOptionsJSON = conv.m_options.ToJSON();

        QFileInfo assetFile(conv.m_sourceAsset);
        const QString folder = assetFile.path();

        QString settingsFileName;

        if (!folder.isEmpty() && folder != ".")
            settingsFileName = folder + "/";

        settingsFileName += assetFile.completeBaseName() + ".ConversionSettings.json";

        QString errorMsg;

        if (!m_storageAccount->CreateTextItem(conv.m_sourceAssetContainer, settingsFileName, advancedOptionsJSON, errorMsg))
        {
            QMessageBox::critical(nullptr, "Starting Conversion Failed", QString("Could not upload the ConversionSettings.json file to the blob storage.\n\nReason: %1").arg(errorMsg), QMessageBox::StandardButton::Ok);
            return false;
        }
    }

    const QString inputSasToken = m_storageAccount->CreateSasToken(conv.m_sourceAssetContainer);
    const QString outputSasToken = m_storageAccount->CreateSasToken(conv.m_outputFolderContainer);

    const QString inputUri = QString("%1/%2").arg(m_storageAccount->GetEndpointUrl()).arg(conv.m_sourceAssetContainer);
    const QString outputUri = QString("%1/%2").arg(m_storageAccount->GetEndpointUrl()).arg(conv.m_outputFolderContainer);

    QString inputFolder = conv.m_inputFolder;

    QString relInputFile = conv.m_sourceAsset;

    if (relInputFile.startsWith(inputFolder))
    {
        relInputFile = relInputFile.mid(inputFolder.length());
    }


    if (inputFolder.endsWith("/"))
    {
        inputFolder.chop(1);
    }

    const QString relInputPath = inputFolder;

    const QString relOutputPath = conv.m_outputFolder;
    const QString relOutputFile = conv.m_name + ".arrAsset";

    conv.m_conversionGuid = QUuid::createUuid().toString(QUuid::WithoutBraces);

    RR::AssetConversionOptions options;
    options.ConversionId = conv.m_conversionGuid.toStdString();

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

    qDebug(LoggingCategory::ArrSdk) << QString("Starting conversion '%1' (%2)").arg(conv.m_name).arg(conv.m_conversionGuid);

    qDebug(LoggingCategory::ArrSdk) << QString("Input Container URI = '%1'").arg(input.StorageContainerUri.c_str());
    qDebug(LoggingCategory::ArrSdk) << QString("Input BlobPrefix = '%1'").arg(input.BlobPrefix.c_str());
    qDebug(LoggingCategory::ArrSdk) << QString("Input Filename = '%1'").arg(input.RelativeInputAssetPath.c_str());
    qDebug(LoggingCategory::ArrSdk) << QString("Input SAS = '%1'").arg(input.StorageContainerReadListSas.c_str());

    qDebug(LoggingCategory::ArrSdk) << QString("Output Container URI = '%1'").arg(output.StorageContainerUri.c_str());
    qDebug(LoggingCategory::ArrSdk) << QString("Output BlobPrefix = '%1'").arg(output.BlobPrefix.c_str());
    qDebug(LoggingCategory::ArrSdk) << QString("Output Filename = '%1'").arg(output.OutputAssetFilename.c_str());
    qDebug(LoggingCategory::ArrSdk) << QString("Output SAS = '%1'").arg(output.StorageContainerWriteSas.c_str());

    auto onConversionStartRequestFinished = [this, conversionIdx](RR::Status status, RR::ApiHandle<RR::AssetConversionResult> result)
    {
        QMetaObject::invokeMethod(QApplication::instance(), [this, conversionIdx, status, result]()
                                  {
                                      RR::Result errorCode = RR::StatusToResult(status);

                                      if (status == RR::Status::OK)
                                      {
                                          errorCode = result->GetErrorCode();
                                      }

                                      auto& conv = m_conversions[conversionIdx];

                                      if (errorCode == RR::Result::Success)
                                      {
                                          std::string conversionUUID;
                                          result->GetConversionUuid(conversionUUID);
                                          conv.m_conversionGuid = conversionUUID.c_str();
                                      }
                                      else
                                      {
                                          conv.m_message = RR::ResultToString(errorCode);
                                          conv.m_status = ConversionStatus::Failed;
                                          conv.m_endConversionTime = QDateTime::currentSecsSinceEpoch();

                                          qCritical(LoggingCategory::ArrSdk) << QString("Starting conversion '%1' failed: %2").arg(conv.m_conversionGuid).arg(conv.m_message);
                                      }

                                      Q_EMIT SelectedChanged();
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
        message = RR::ResultToString(RR::StatusToResult(status));
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
            qCritical(LoggingCategory::ArrSdk) << QString("Conversion '%1' failed: %2").arg(conv.m_conversionGuid).arg(conv.m_message);
            Q_EMIT ConversionFailed();
            break;

        case RR::ConversionSessionStatus::Success:
            conv.m_status = ConversionStatus::Finished;
            conv.m_endConversionTime = QDateTime::currentSecsSinceEpoch();
            Q_EMIT ConversionSucceeded();
            break;
    }
}

QString Conversion::GetPlaceholderName() const
{
    return QFileInfo(m_sourceAsset).baseName();
}

QString Conversion::GetPlaceholderInputFolder() const
{
    QString srcFolder = m_sourceAsset;
    int lastSlash = srcFolder.lastIndexOf("/");
    if (lastSlash >= 0)
    {
        return srcFolder.left(lastSlash + 1);
    }

    return "";
}
